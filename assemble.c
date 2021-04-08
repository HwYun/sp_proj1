#include "20161615.h"

int sym_cur = 0;

int find_sym(char *label){
	for (int i=0 ; i<100; i++){
		if(strcmp(symbol_tab[i].symbol, label) == 0) return i; // 해당 label을 찾았을 경우
	}
	return -1; // 해당 label을 못 찾았을 경우
}

int pass_one(char *filename, int *length){

	if (strcmp(filename + strlen(filename) - 4, ".asm") != 0) return -1; // asm 파일이 아닐 경우 바로 종료

	FILE *fp = fopen(filename, "r");

	int LOCCTR=0;
	int start_address = 0;
	char asm_line[100];
	int file_line_num = 0;
	char label[20];
	char opcode[20];
	char operand[20];
	int operand_val;



	while(1){
		file_line_num += 5;
		for(int i=0 ; i<100 ; i++) asm_line[i] = 0;
		for(int i=0 ; i<20; i++){
			label[i] = 0;
			opcode[i] = 0;
			operand[i] = 0;
		}
		if(fgets(asm_line, 100, fp) == NULL) break;
		asm_line[strlen(asm_line)-1] = '\0';
		replace(asm_line, 9, 32);
		replace(asm_line, 13, 32);
		strncpy(label, asm_line, 7);
		label[6] = 0;
		strncpy(opcode, asm_line + 7, 7);
		opcode[6] = 0;
		for(int i=0 ; i<7 ; i++){
			if(label[i] == ' ') label[i] = '\0';
			if(opcode[i] == ' ') opcode[i] = '\0';
		}
		strcpy(operand, asm_line + 14);
		// printf("label: %s\t opcode: %s\t operand: %s\n", label, opcode, operand); // 잘 나뉘어졌는지 확인.

		if( strcmp(opcode, "START") == 0){
			LOCCTR = hexadecimal_to_decimal(operand);
			if(LOCCTR == -1){
				error_handling(file_line_num);
				return -1;
			}
			start_address = LOCCTR;
		}
		else if (strcmp(label, ".") != 0){
			if(find_sym(label) == -1){
				strcpy(symbol_tab[sym_cur].symbol, label);
				symbol_tab[sym_cur].location = LOCCTR;
				sym_cur++;
			}
			else if(strlen(label) > 1){
				error_handling(file_line_num);
				return -1;
			}
			if (opcode[0] == '+'){
				char tmp_str[10];
				strcpy(tmp_str, opcode + 1);
				strcpy(opcode, tmp_str);
				LOCCTR += 1;
			}
			if( find_hash(opcode) == -1){ // optable에 해당 opcode가 없을 때
				if(strcmp(opcode, "WORD") == 0) {
					operand_val = atoi(operand);
					LOCCTR += 3;
				}
				else if(strcmp(opcode, "RESW") == 0){
					operand_val = atoi(operand);
					LOCCTR += 3*operand_val;
				}
				else if(strcmp(opcode, "RESB") == 0){
					operand_val = atoi(operand);
					LOCCTR += operand_val;

				}
				else if(strcmp(opcode, "BYTE") == 0){
					// operand의 길이를 계산하여 LOCCTR에 더해준다.
					char *tmp_ptr;
					char data_type[2];
					char operand_data[10];

					tmp_ptr = strtok(operand, "'");
					strcpy(data_type, tmp_ptr);

					tmp_ptr = strtok(NULL, "'");
					strcpy(operand_data, tmp_ptr);
					if(strcmp(data_type, "C") == 0){ // 문자열
						LOCCTR += strlen(operand_data);
					}
					else if(strcmp(data_type, "X") == 0){ // 16진수
						LOCCTR += strlen(operand_data)/2;
						operand_val = hexadecimal_to_decimal(operand_data);
					}
					else{
						error_handling(file_line_num);
						return -1;
					}
				}
				else if(strcmp(opcode, "END") == 0 ){
					*length = LOCCTR - start_address;
					break;
				}
				else if(strcmp(opcode, "BASE") == 0){
					// pass
				}
				else{
					sym_cur = 0;
					printf("It is not the opcode: %s, line# %d\n", opcode, file_line_num);
					return -1;
				}
			}
			else{ // 올바른 opcode 입력
				int key = hash_key(opcode);
				char format[5];
				HashNode* tmp = NULL;
				for (tmp = hash_table[key] ; tmp != NULL ; tmp = tmp->next){
					if(strcmp(tmp->mnemonic, opcode) == 0)
						strcpy(format, tmp->format);
				}
				if(strcmp(format, "1") == 0)
					LOCCTR += 1;
				else if(strcmp(format, "2") == 0)
					LOCCTR += 2;
				else if (strcmp(format, "3/4") == 0)
					LOCCTR += 3;
			}
		}
	}
	if (sym_cur != 0){
		for (int i=0 ; i< 100 ; i++)
			last_symbol_tab[i] = symbol_tab[i];
	}

	fclose(fp);
	return file_line_num; // 올바르게 수행되었을 시 총 line num을 반환.
}

int pass_two(char *filename, int file_line_len, int length){
	if (strcmp(filename + strlen(filename) - 4, ".asm") != 0) return -1; // asm 파일이 아닐 경우 바로 종료
	if (file_line_len <= 0) return -1;

	FILE *fp = fopen(filename, "r");

	int line_len = file_line_len/5;
	SrcState *stat = (SrcState *)malloc(sizeof(SrcState)*(line_len + 2));
	
	for (int i=0 ; i<line_len ; i++){ // initialize
		stat[i].loc = 0;
		for(int j=0 ; j<20; j++){
			stat[i].label[j] = 0;
			stat[i].opcode[j] = 0;
			stat[i].operand[j] = 0;
			stat[i].obj_code[j/2] = 0;
		}
		for (int j=0 ; j<100 ; j++){
			stat[i].comment[j] = 0;
		}
	}

	char n, i_reg, x, b, p, e;
	int pc, base;
	char opc;
	int address = 0;
	int LOCCTR=0;
	char asm_line[100];
	int file_line_num = 0;
	char label[20];
	char opcode[20];
	char operand[20];
	int sym_idx = 0;
	int cons_flag = 0;

	while(1){
		cons_flag = 0;
		file_line_num += 5;
		int stat_idx = file_line_num/5;
		n = 1, i_reg = 1, x = 0, b = 0, p = 0, e = 0;
		for(int i=0 ; i<100 ; i++) asm_line[i] = 0;
		for(int i=0 ; i<20; i++){
			label[i] = 0;
			opcode[i] = 0;
			operand[i] = 0;
		}
		if(fgets(asm_line, 100, fp) == NULL) break;
		asm_line[strlen(asm_line)-1] = '\0';
		replace(asm_line, 9, 32);
		replace(asm_line, 13, 32);
		strncpy(label, asm_line, 7);
		label[6] = 0;
		strncpy(opcode, asm_line + 7, 7);
		opcode[6] = 0;
		for(int i=0 ; i<7 ; i++){
			if(label[i] == ' ') label[i] = '\0';
			if(opcode[i] == ' ') opcode[i] = '\0';
		}
		strcpy(operand, asm_line + 14);
		// printf("label: %s\t opcode: %s\t operand: %s\n", label, opcode, operand); // 잘 나뉘어졌는지 확인.

		if (opcode[0] == '+'){ // 4형식인지 체크
			char tmp_str[20];
			strcpy(tmp_str, opcode + 1);
			strcpy(opcode, tmp_str);
			e = 1;
		}

		if (operand[0] == '@'){ // indirect인지 체크
			char tmp_str[20];
			strcpy(tmp_str, operand + 1);
			strcpy(operand,tmp_str);
			n = 1, i_reg = 0;
		}
		else if (operand[0] == '#'){ // immediate인지 체크
			char tmp_str[20];
			strcpy(tmp_str, operand + 1);
			strcpy(operand, tmp_str);
			n = 0, i_reg = 1;
		}

		if( strcmp(opcode, "START") == 0){
			LOCCTR = hexadecimal_to_decimal(operand);
			stat[stat_idx].loc = LOCCTR;
			strcpy(stat[stat_idx].label, label);
			strcpy(stat[stat_idx].opcode, opcode);
			strcpy(stat[stat_idx].operand, operand);
		}
		else if(strcmp(opcode, "END") == 0){
			stat[stat_idx].loc = LOCCTR;
			strcpy(stat[stat_idx].label, label);
			strcpy(stat[stat_idx].opcode, opcode);
			strcpy(stat[stat_idx].operand, operand);
			break;
		}
		else if (strcmp(label, ".") != 0){ // comment가 아니라면
			opc = find_hash(opcode);
			stat[stat_idx].loc = LOCCTR;
			strcpy(stat[stat_idx].label, label);
			strcpy(stat[stat_idx].opcode, opcode);
			strcpy(stat[stat_idx].operand, operand);
			char chk_format[5] = {0,};
			if(opc != -1){ // OPTAB 에 해당 mnemonic이 있다면
				if( e == 1 ) LOCCTR += 1;

				find_format(opcode, chk_format);
				// printf("opcode: %s\t format: %s\n", opcode, chk_format);

				if(strcmp(chk_format, "1") == 0) LOCCTR += 1;
				else if(strcmp(chk_format, "2") == 0) LOCCTR += 2;
				else if(strcmp(chk_format, "3/4") == 0) LOCCTR += 3;

				if(strcmp(chk_format, "2") == 0){ // 2형식일 경우
					char *chk_ptr = strchr(operand, ',');
					if (strlen(operand) == 0){
						error_handling(file_line_num);
						return -1;
					}
					if(chk_ptr == NULL){ // operand가 하나
						if(strcmp(operand, "A") == 0) address = 0;
						else if(strcmp(operand, "X") == 0) address = 1 * 16;
						else if(strcmp(operand, "L") == 0) address = 2 * 16;
						else if(strcmp(operand, "B") == 0) address = 3 * 16;
						else if(strcmp(operand, "S") == 0) address = 4 * 16;
						else if(strcmp(operand, "T") == 0) address = 5 * 16;
						else if(strcmp(operand, "F") == 0) address = 6 * 16;
						else if(strcmp(operand, "PC") == 0) address = 8 * 16;
						else if(strcmp(operand, "SW") == 0) address = 9 * 16;
					}
					else{ // operand가 2개
						char first_operand[5];
						char second_operand[5];
						char *tok_ptr;
						tok_ptr = strtok(operand, ",");
						strcpy(first_operand, tok_ptr);
						tok_ptr = strtok(NULL, " ");
						strcpy(second_operand, tok_ptr);
						if(strcmp(first_operand, "A") == 0) address = 0;
						else if(strcmp(first_operand, "X") == 0) address = 1 * 16;
						else if(strcmp(first_operand, "L") == 0) address = 2 * 16;
						else if(strcmp(first_operand, "B") == 0) address = 3 * 16;
						else if(strcmp(first_operand, "S") == 0) address = 4 * 16;
						else if(strcmp(first_operand, "T") == 0) address = 5 * 16;
						else if(strcmp(first_operand, "F") == 0) address = 6 * 16;
						else if(strcmp(first_operand, "PC") == 0) address = 8 * 16;
						else if(strcmp(first_operand, "SW") == 0) address = 9 * 16;
						
						if(strcmp(second_operand, "A") == 0) address += 0;
						else if(strcmp(second_operand, "X") == 0) address += 1;
						else if(strcmp(second_operand, "L") == 0) address += 2;
						else if(strcmp(second_operand, "B") == 0) address += 3;
						else if(strcmp(second_operand, "S") == 0) address += 4;
						else if(strcmp(second_operand, "T") == 0) address += 5;
						else if(strcmp(second_operand, "F") == 0) address += 6;
						else if(strcmp(second_operand, "PC") == 0) address += 8;
						else if(strcmp(second_operand, "SW") == 0) address += 9;
						

					}
					address += opc*16*16;
					char tmp_hexa[10];
					sprintf(tmp_hexa, "%0X", address);
					strcpy(stat[stat_idx].obj_code, tmp_hexa + 4);

				}
				else if(strcmp(chk_format, "3/4") == 0){ // 3 또는 4형식일 경우
					if(strlen(operand) == 0) {
						n = 1; i_reg = 1;
						x = 0; b = 0; p = 0; e = 0;
						address = 0;
					}
					else{

						sym_idx = find_sym(operand);
						if(sym_idx == -1){ // SYMTAB에 해당 operand가 없을 경우
							char *chk_ptr = strchr(operand, ',');
							if(chk_ptr == NULL){ // operand가 하나
								for(int i=0 ; i<strlen(operand) ; i++){
									if((operand[i] < '0') || (operand[i] > '9')){
										error_handling(file_line_num);
										return -1;
									}
								}
								address = atoi(operand);
								cons_flag = 1;
							}
							else{ // 사실상 인덱스 relative
								char first_operand[20];
								char second_operand[20];
								char *tok_ptr;
								tok_ptr = strtok(operand, ",");
								strcpy(first_operand, tok_ptr);
								tok_ptr = strtok(NULL, " ");
								strcpy(second_operand, tok_ptr);

								//printf("first: %s\n, seconde: %s\n", first_operand, second_operand);
								sym_idx = find_sym(first_operand);
								if(sym_idx == -1){
									error_handling(file_line_num);
									return -1;
								}
								address = symbol_tab[sym_idx].location;
								if(strcmp(second_operand, "X") != 0){
									error_handling(file_line_num);
									return -1;
								}
								x = 1; // index reg set
							}
						}
						else{ // SYMTAB에 operand가 있을 경우
							address = symbol_tab[sym_idx].location;
						}
					}
					pc = LOCCTR;
					int xbpe = 0;
					char temp_str[20] = {0,};
					if( n == 1 && i_reg == 1) opc += 3; // simple address
					else if( n==1 && i_reg == 0) {
						opc += 2; // indirect address
						strcpy(temp_str, stat[stat_idx].operand);
						sprintf(stat[stat_idx].operand, "@%s", temp_str);
					}
					else if( n==0 && i_reg == 1){
						opc += 1; // immediate address
						strcpy(temp_str, stat[stat_idx].operand);
						sprintf(stat[stat_idx].operand, "#%s", temp_str);
					}

					if( e == 1 ){ // format 4
						b = 0; p = 0;
						xbpe = x * my_pow(2, 3) + b * my_pow(2, 2) + p * 2 + e;

						address += opc*my_pow(16,6);
						address += xbpe*my_pow(16,5);
						char tmp_hexa[10];
						sprintf(tmp_hexa, "%08X", address);
						strcpy(stat[stat_idx].obj_code, tmp_hexa);

						strcpy(temp_str, stat[stat_idx].opcode);
						sprintf(stat[stat_idx].opcode, "+%s", temp_str);

					}
					else if(strlen(operand) == 0 ){
						address += opc * my_pow(16,4);
						char tmp_hexa[10];
						sprintf(tmp_hexa, "%08X", address);
						strcpy(stat[stat_idx].obj_code, tmp_hexa + 2);

					}
					else if( n==0 && i_reg * cons_flag == 1 ){
						address += opc * my_pow(16,4);
						char tmp_hexa[10];
						sprintf(tmp_hexa, "%08X", address);
						strcpy(stat[stat_idx].obj_code, tmp_hexa + 2);
					}
					else if( (address - pc) < -2048 || (address - pc) > 2047){ // base relative
						// pc relative로는 너무 차이가 클 때
						b = 1; p = 0; 
						// base relative는 disp이 항상 0 이상의 정수이기 때문에 음수 고려 X
						address -= base;
						xbpe = x * my_pow(2, 3) + b * my_pow(2, 2) + p * 2;

						address += opc * my_pow(16,4);
						address += xbpe * my_pow(16,3);
						char tmp_hexa[10];
						sprintf(tmp_hexa, "%08X", address);
						strcpy(stat[stat_idx].obj_code, tmp_hexa + 2);

					}
					else{ // pc relative
						b = 0; p = 1;
						xbpe = x * my_pow(2, 3) + b * my_pow(2, 2) + p * 2;
						// address가 음수가 되는 것도 고려 해야함.
						address -= pc;
						
						if(address >=0){ 
							address += opc * my_pow(16,4);
							address += xbpe * my_pow(16,3);
							char tmp_hexa[10];
							sprintf(tmp_hexa, "%08X", address);
							strcpy(stat[stat_idx].obj_code, tmp_hexa + 2);
						}
						else{ // address - pc 값이 음수인 경우. 2's complement를 해야함.
							char tmp_hexa[10];
							char tmp_str[10];
							// printf("address: %4X, pc: %4X, %X\n", address + pc , pc, address);
							sprintf(tmp_hexa, "%X", address);
							strcpy(tmp_str, tmp_hexa + 5);

							address = hexadecimal_to_decimal(tmp_str);
							address += opc * my_pow(16,4);
							address += xbpe * my_pow(16,3);
							sprintf(tmp_hexa, "%08X", address);
							strcpy(stat[stat_idx].obj_code, tmp_hexa + 2);

						}

					}

				}
			}
			else { // mnemonic이 OPTAB에 없는 경우
				if(strcmp(opcode, "WORD") == 0){
					// WORD	
					LOCCTR += 3;
					sym_idx=find_sym(label);
					address = atoi(operand);
					char tmp_hexa[10];
					sprintf(tmp_hexa, "%X", address);
					strcpy(stat[stat_idx].obj_code, tmp_hexa + 2);
				}
				else if(strcmp(opcode, "BYTE") == 0){
					// BYTE operand의 길이를 계산하여 LOCCTR에 더해준다.
					sym_idx = find_sym(label);
					char *tmp_ptr;
					char data_type[2];
					char operand_data[10];

					tmp_ptr = strtok(operand, "'");
					strcpy(data_type, tmp_ptr);

					tmp_ptr = strtok(NULL, "'");
					strcpy(operand_data, tmp_ptr);
					address = 0;
					if(strcmp(data_type, "C") == 0){ // 문자열
						int data_len = strlen(operand_data);
						LOCCTR += data_len;
						for ( int i=0 ; i < data_len ; i++ ){
							address += operand_data[i] * my_pow(16, 2*(data_len - i - 1));  
						}
						char tmp_hexa[10];
						sprintf(tmp_hexa, "%0X", address);
						strcpy(stat[stat_idx].obj_code, tmp_hexa );

					}
					else if(strcmp(data_type, "X") == 0){ // 16진수
						LOCCTR += 1;
						address = hexadecimal_to_decimal(operand_data);
						char tmp_hexa[10];
						sprintf(tmp_hexa, "%02X", address);
						strcpy(stat[stat_idx].obj_code, tmp_hexa);
					}
				}
				else if(strcmp(opcode, "RESW") == 0){
					// RESW
					LOCCTR += 3*atoi(operand);
				}
				else if(strcmp(opcode, "RESB") == 0){
					// RESB
					LOCCTR += atoi(operand);
				}
				else if(strcmp(opcode, "BASE") == 0){
					// BASE
					sym_idx = find_sym(operand);
					base = symbol_tab[sym_idx].location;
				}
			}
		}
		else{ // comment 인 경우
			asm_line[0] = ' ';
			strcpy(stat[stat_idx].label, label);
			strcpy(stat[stat_idx].comment, asm_line);
		}

	}
	fclose(fp);
	// 여기까지 stat 구조체 생성 완료.

	// 여기부터 obj 파일 및 lst 파일 생성. 
	// 오류가 발생했다면 이 함수는 진작 종료되기 때문에 여기서부터는 오류 고려 하지 않음.

	generate_obj_lst(stat, filename, length);
	free(stat);
	return 0;

}


void generate_obj_lst(SrcState *stat, char *filename, int length){
	char *tmp_ptr= strtok(filename, ".");
	char file[10];
	strcpy(file, tmp_ptr);
	char filename_lst[20], filename_obj[20];
	sprintf(filename_lst, "%s.lst", file);
	sprintf(filename_obj, "%s.obj", file);

	printf("[%s], [%s]\n", filename_lst, filename_obj);

	FILE *lst_fp = fopen(filename_lst, "w");
	FILE *obj_fp = fopen(filename_obj, "w");

	int idx = 1;
	while(1){ // listing file 생성
		if(strcmp(stat[idx].label, ".") == 0){
			fprintf(lst_fp, "%-d\t\t%s\n", idx*5, stat[idx].comment);
		}
		else if(strcmp(stat[idx].opcode, "END") == 0 || strcmp(stat[idx].opcode, "BASE") == 0){
			fprintf(lst_fp, "%-d\t\t%s\t%s\t%s\n", idx*5, stat[idx].label,stat[idx].opcode,stat[idx].operand);
		}
		else{
			fprintf(lst_fp, "%-d\t%04X\t%s\t%s", idx*5, stat[idx].loc, stat[idx].label, stat[idx].opcode);
			if(strlen(stat[idx].operand) > 7)
				fprintf(lst_fp, "\t%s\t%s\n", stat[idx].operand, stat[idx].obj_code);
			else
				fprintf(lst_fp, "\t%s\t\t%s\n", stat[idx].operand, stat[idx].obj_code);
		}
		if(strcmp(stat[idx].opcode, "END") == 0) break;
		idx++;
	}
	fclose(lst_fp); // listing file 생성 끝

	idx = 1;
	int newline_flag = 0; // 이전에 개행됐으면 1
	int modify_loc[100] = {0,};
	int modify_pos=0;
	int total_len = 0;
	while(1){
		if(idx == 1) {
			fprintf(obj_fp, "H%-6s%06X%06X\n",stat[idx].label,hexadecimal_to_decimal(stat[idx].operand),length);
			newline_flag = 1;
			idx++;
		}
		else{
			int object_len = 0;
			char object[100] = {0,};
			while(1){
				if(newline_flag == 1){
					if(strlen(stat[idx].obj_code) > 0){
						newline_flag = 0;
						fprintf(obj_fp, "T%06X", stat[idx].loc);
						object_len += strlen(stat[idx].obj_code);
						total_len += strlen(stat[idx].obj_code);
						strcat(object, stat[idx].obj_code);
					}
				}
				else {
					object_len += strlen(stat[idx].obj_code);
					total_len += strlen(stat[idx].obj_code);
					if(strcmp(stat[idx].opcode, "RESW") == 0 || strcmp(stat[idx].opcode, "RESB") == 0){
						newline_flag = 1;
						fprintf(obj_fp, "%02X%s\n", object_len/2, object);
						break;
					}
					else if(object_len > 60 || strcmp(stat[idx].opcode, "END") == 0){
						object_len -= strlen(stat[idx].obj_code);
						total_len -= strlen(stat[idx].obj_code);
						newline_flag = 1;
						fprintf(obj_fp, "%02X%s\n", object_len/2, object);
						break;
					}
					else{
						if(strlen(stat[idx].obj_code) == 8){
							if(stat[idx].operand[0] != '#'){
								modify_loc[modify_pos++] = (total_len - 6) / 2;
							}
						}
						strcat(object, stat[idx].obj_code);
					}
				}
				idx++;
			}
		}
		if(strcmp(stat[idx].opcode, "END") == 0) {
			for(int i=0 ; i<modify_pos ; i++){
				fprintf(obj_fp, "M%06X05\n", modify_loc[i]);
			}
			fprintf(obj_fp, "E%06X\n", hexadecimal_to_decimal(stat[1].operand));
			break;
		}
	}
	fclose(obj_fp);
}

void error_handling(int line_num){
	sym_cur = 0;
	printf("There is an Error: line# %d\n", line_num);
}
void print_symbol_table(){

	char tmp_symbol[10];
	int tmp_location;


	for (int i=0 ; i< sym_cur - 1 ; i++){
		for (int j = i + 1 ; j < sym_cur ; j++){
			if(strcmp(last_symbol_tab[i].symbol, last_symbol_tab[j].symbol) >= 1){

				strcpy(tmp_symbol, last_symbol_tab[i].symbol);
				tmp_location = last_symbol_tab[i].location;

				strcpy(last_symbol_tab[i].symbol, last_symbol_tab[j].symbol);
				last_symbol_tab[i].location = last_symbol_tab[j].location;

				strcpy(last_symbol_tab[j].symbol, tmp_symbol);
				last_symbol_tab[j].location = tmp_location;
			}
		}
	}

	for(int i=0 ; i<sym_cur ; i++)
		printf("\t%s\t%04X\n", last_symbol_tab[i].symbol, last_symbol_tab[i].location);
}
