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
					symbol_tab[sym_cur].num_data = operand_val;
				}
				else if(strcmp(opcode, "RESW") == 0){
					operand_val = atoi(operand);
					LOCCTR += 3*operand_val;
					symbol_tab[sym_cur].num_data = operand_val;
				}
				else if(strcmp(opcode, "RESB") == 0){
					operand_val = atoi(operand);
					LOCCTR += operand_val;
					symbol_tab[sym_cur].num_data = operand_val;

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
						strcpy(symbol_tab[sym_cur].str_data, operand_data);
						symbol_tab[sym_cur].num_data = -1;
					}
					else if(strcmp(data_type, "X") == 0){ // 16진수
						LOCCTR += strlen(operand_data)/2;
						operand_val = hexadecimal_to_decimal(operand_data);
						symbol_tab[sym_cur].num_data = operand_val;
						symbol_tab[sym_cur].str_data[0] = 0;
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

	FILE *asm_fp = fopen(filename, "r");

	int line_len = file_line_len/5;
	SrcState *stat = (SrcState *)malloc(sizeof(SrcState)*(line_len));
	
	for (int i=0 ; i<line_len ; i++){ // initialize
		stat[i].loc = 0;
		for(int j=0 ; j<20; j++){
			stat[i].label[j] = 0;
			stat[i].opcode[j] = 0;
			stat[i].operand[j] = 0;
			stat[i].operand[j/2] = 0;
		}
		for (int j=0 ; j<100 ; j++){
			stat[i].comment[j] = 0;
		}
	}

	char n, i_reg, x, b, p, e;
	int pc, base;
	char opc;
	char base_sym[20];
	int address = 0;
	int LOCCTR=0;
	int start_address = 0;
	char asm_line[100];
	int file_line_num = 0;
	char label[20];
	char opcode[20];
	char operand[20];
	int operand_val;
	int sym_idx = 0;



	while(1){
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
			char chk_format[5];
			if(opc != -1){ // OPTAB 에 해당 mnemonic이 있다면
				if( e == 1 ) LOCCTR += 1;

				find_format(opcode, chk_format);

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
						else if(strcmp(operand, "L") == 0) adderss = 2 * 16;
						else if(strcmp(operand, "B") == 0) address = 3 * 16;
						else if(strcmp(operand, "S") == 0) address = 4 * 16;
						else if(strcmp(operand, "T") == 0) address = 5 * 16;
						else if(strcmp(operand, "F") == 0) address = 6 * 16;
						else if(strcmp(operand, "PC") == 0) address = 8 * 16;
						else if(strcmp(operand, "SW") == 0) address = 9 * 16;
					}
					else{ // operand가 2개
						char first_operand[3];
						char second_operand[3];
						*chk_ptr = ' ';
						char *tok_ptr;
						first_operand = strtok(operand, ' ');
						second_operand = strtok(NULL, ' ');
						if(strcmp(first_operand, "A") == 0) address = 0;
						else if(strcmp(first_operand, "X") == 0) address = 1 * 16;
						else if(strcmp(first_operand, "L") == 0) adderss = 2 * 16;
						else if(strcmp(first_operand, "B") == 0) address = 3 * 16;
						else if(strcmp(first_operand, "S") == 0) address = 4 * 16;
						else if(strcmp(first_operand, "T") == 0) address = 5 * 16;
						else if(strcmp(first_operand, "F") == 0) address = 6 * 16;
						else if(strcmp(first_operand, "PC") == 0) address = 8 * 16;
						else if(strcmp(first_operand, "SW") == 0) address = 9 * 16;
						
						if(strcmp(second_operand, "A") == 0) address += 0;
						else if(strcmp(second_operand, "X") == 0) address += 1;
						else if(strcmp(second_operand, "L") == 0) adderss += 2;
						else if(strcmp(second_operand, "B") == 0) address += 3;
						else if(strcmp(second_operand, "S") == 0) address += 4;
						else if(strcmp(second_operand, "T") == 0) address += 5;
						else if(strcmp(second_operand, "F") == 0) address += 6;
						else if(strcmp(second_operand, "PC") == 0) address += 8;
						else if(strcmp(second_operand, "SW") == 0) address += 9;
						
						address += (int)opc*16*16;
						decimal_to_hexadecimal(address, stat[stat_idx].obj_code);
					}
				}
				else if(strcmp(chk_format, "3/4") == 0){ // 3 또는 4형식일 경우
					if(strlen(operand) == 0) address = 0;
					else{
						sym_idx = find_sym(operand);
						if(sym_idx == -1){ // SYMTAB에 해당 operand가 없을 경우
							char *chk_ptr = strchr(operand, ',');
							if(chk_ptr == NULL){ // operand가 하나
								for(int i=0 ; i<strlen(operand) ; i++){
									if((operand[i] < '0') || (operand > '9')){
										error_handling(file_line_num);
										return -1;
									}
								}
								address = atoi(operand);
							}
							else{ // 사실상 인덱스 relative
								char first_operand[20];
								char second_operand[20];
								*chk_ptr = ' ';
								char *tok_ptr;
								first_operand = strtok(operand, ' ');
								second_operand = strtok(NULL, ' ');

								sym_idx = find_sym(first_operand);
								if(sym_idx == -1){
									error_handling(file_line_num);
									return -1;
								}
								address = sym_tab[sym_idx].location;
								if(strcmp(second_operand, "X") != 0){
									error_handling(file_line_num);
									return -1;
								}
								x = 1; // index reg set
							}
						}
						else{ // SYMTAB에 operand가 있을 경우
							address = sym_tab[sym_idx].location;
						}
					}
					pc = LOCCTR;
					int xbpe = 0;
					if( n == 1 && i_reg == 1) opc += 3; // simple address
					else if( n==1 && i_reg == 0) opc += 2; // indirect address
					else if( n==0 && i_reg == 1) opc += 1; // immediate address

					if( e == 1 ){ // format 4
						b = 0; p = 0;
						xbpe = x * my_pow(2, 3) + b * my_pow(2, 2) + p * 2 + e;

						address += opc*my_pow(16,6);
						address += xbpe*my_pow(16,5);
						decimal_to_hexadecimal(address, stat[stat_idx].obj_code);
					}
					else if( (address - pc) < -2048 || (address - pc) > 2047){ // base relative
						// pc relative로는 너무 차이가 클 때
						b = 1; p = 0; 
						// base relative는 disp이 항상 0 이상의 정수이기 때문에 음수 고려 X
						address -= base;
						xbpe = x * my_pow(2, 3) + b * my_pow(2, 2) + p * 2;

						address += opc * my_pow(16,4);
						address += xbpe * my_pow(16,3);
						decimal_to_hexadecimal(address, stat[stat_idx].obj_code);
					}
					else{ // pc relative
						b = 0; p = 1;
						xbpe = x * my_pow(2, 3) + b * my_pow(2, 2) + p * 2;
						// address가 음수가 되는 것도 고려 해야함.
						address -= pc;
						
						if(address >=0){ 
							address += opc * my_pow(16,4);
							address += xbpe * my_pow(16,3);
							decimal_to_hexadecimal(address, stat[stat_idx].obj_code);
						}
						else{ // address - pc 값이 음수인 경우. 2's complement를 해야함.
							char tmp_hexa[10];
							char tmp_str[10];
							sprintf(tmp_haxa, "%X", address - pc);
							strcpy(tmp_str, tmp_hexa + 5);

							address = hexadecimal_to_decimal(tmp_str);
							address += opc * my_pow(16,4);
							address += xbpe * my_pow(16,3);
							decimal_to_hexadecimal(address, stat[stat_idx].obj_code);
						}
					}
				}
			}
			else { // mnemonic이 OPTAB에 없는 경우
				if(strcmp(opcode, "WORD") == 0){
					// WORD	
					LOCCTR += 3;
					sym_idx=find_sym(label);
					address = sym_tab[sym_idx].num_data;
					char tmp_hexa[10];
					sprintf(tmp_haxa, "%X", address);
					strcpy(stat[stat_idx].obj_code, tmp_hexa + 2);
				}
				else if(strcmp(opcode, "BYTE") == 0){
					// BYTE operand의 길이를 계산하여 LOCCTR에 더해준다.
					sym_idx = find_sym(label);

					if(symbol_tab[sym_idx].num_data == -1){ // 문자열
						int data_len = strlen(symbol_tab[sym_idx].str_data);
						LOCCTR += data_len;
						for ( int i=0 ; i < data_len ; i++ ){
							address += (int)(symbol_tab[sym_idx].str_data[i]) * my_pow(16, data_len - i - 1);  
						}
						decimal_to_hexadecimal(address, stat[stat_idx].obj_code);
					}
					else if(symbol_tab[sym_idx].num_data > -1){ // 16진수
						LOCCTR += 1
						address = symbol_tab[sym_idx].num_data;
						char tmp_hexa[10];
						sprintf(tmp_hexa, "%X", address);
						strcpy(stat[stat_idx].obj_code, tmp_hexa + 6);
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
			strcpy(stat[stat_idx].comment, asm_line);
		}
	}
	fclose(fp);
	// 여기까지 stat 구조체 생성 완료.

	// 여기부터 obj 파일 및 lst 파일 생성. 
	// 오류가 발생했다면 이 함수는 진작 종료되기 때문에 여기서부터는 오류 고려 하지 않음.

	generate_obj_lst(stat, filename);
	return 0;

}

void decimal_to_hexadecimal(int decimal, char *hexadecimal){ // 10진수를 16진수 문자열로 바꿔줌.
	int pos = 0, mod = 0; 
	int dec = decimal;
	char hex[strlen(hexadecimal)];
	while(1){
		mod = dec % 16;
		if (mod < 10)
			hex[pos] = '0' + (char)mod;
		else
			hex[pos] = 'A' - 10 + (char)mod;
		dec /= 16;
		pos++;
		if(dec == 0 && pos == 4) break; // format 2
		if(dec == 0 && pos == 6) break; // format 3
		if(dec == 0 && pos == 8) break; // format 4
	}

	for (int i=0 ; i<pos ; i++)
		hexadecimal[i] = hex[pos - i - 1];

	hexadecimal[pos] = '\0';
}

void generate_obj_lst(SrcState stat, char *filename){
	// 추후 구현 예정
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
