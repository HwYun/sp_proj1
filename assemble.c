#include "20161615.h"

int sym_cur = 0;

int find_sym(char *label){
	for (int i=0 ; i<100; i++){
		if(strcmp(symbol_tab[i].symbol, label) == 0) return 1; // 해당 label을 찾았을 경우
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
				break;
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
				break;
			}
			if (opcode[0] == '+'){
				char tmp_str[10];
				strcpy(tmp_str, opcode + 1);
				strcpy(opcode, tmp_str);
				LOCCTR += 1;
			}
			if( find_hash(opcode) == -1){ // optable에 해당 opcode가 없을 때
				if(strcmp(opcode, "WORD") == 0) LOCCTR += 3;
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
					}
					else{
						error_handling(file_line_num);
						break;
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
					break;
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

	return 0;
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
