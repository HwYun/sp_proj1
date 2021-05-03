#include "20161615.h"

typedef struct _history* hisptr; // History linked list를 만들기 위한 history 포인터

typedef struct _history{ // 명령어 History 구조체
	char ins_token[6][INSTRUCTION_LEN]; // 명령어 토큰은 최대 6개 (fill start, end, value)
	hisptr hisrear; // 다음 History 노드로 이어주는 링크
}his;



void create_history_ins(char *instruction);
void create_history_tok(char token[10][INSTRUCTION_LEN]);
void print_history();
void free_history();


hisptr front = NULL;
hisptr rear = NULL;

int main(){ 

	int dump_address = 0;
	create_opcode_hash();

	int asm_length=0;
	while(1){
		printf("sicsim> ");
		char instruction[INSTRUCTION_LEN] = {0, };
		char tokenize[INSTRUCTION_LEN] = {0, };
		fgets(instruction, INSTRUCTION_LEN, stdin);
		replace(instruction, '\t', ' ');
		instruction[strlen(instruction)-1]='\0';
		strcpy(tokenize, instruction);
		// printf("%s\n",instruction); // Test Input
		// printf("%d\n", (int)strlen(instruction));

		int end_idx = 0;
		char token[10][INSTRUCTION_LEN];
		for(int i=0 ; i<10; i++)
			for(int j=0 ; j<INSTRUCTION_LEN;j++)
				token[i][j] = 0;
		if(strchr(tokenize, 32)) {
			char *temp_tok = strtok(tokenize, " ");
			strcpy(token[0], temp_tok);
			for (int i=1 ; i<10 ; i++){
				temp_tok = strtok(NULL, " ");
				if(temp_tok == NULL) {
					end_idx = i;
					break;
				}
				strcpy(token[i], temp_tok);
			}
			// printf("공백 있음!!\n");
		}
		if (end_idx == 1){
			strcpy(instruction, token[0]);
		}
		// else strcpy(token[0], instruction);
		/*
		for(int i=0;i<end_idx;i++){
			if(token[i]==NULL) break;
			printf("%s\n",token[i]);
		}
		*/
		if (strcmp(instruction, "quit") == 0 || strcmp(instruction, "q") == 0) break; // quit
		else if (strcmp(instruction, "help") == 0 || strcmp(instruction, "h") == 0){ // help
			printf("\nh[elp]\nd[ir]\nq[uit]\nhi[story]\ndu[mp] [start, end]\n");
			printf("e[dit] address, value\nf[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\n");
			printf("assemble filename\ntype filename\nsymbol\n");
			printf("progaddr [address]\nloader [object filename1] [object filename2] [...]\n");
			printf("bp [address]\nrun\n\n");
			create_history_ins(instruction);
		}
		else if(strcmp(instruction, "dir") == 0 || strcmp(instruction, "d") == 0){ // dir
			instruction_dir();
			create_history_ins(instruction);
		}
		else if(strcmp(instruction, "history") == 0 || strcmp(instruction, "hi") == 0){ // history
			create_history_ins(instruction);
			print_history();
		}
		else if(strcmp(instruction, "du") == 0 || strcmp(instruction, "dump") == 0){
			// dump 단일 명령어
			create_history_ins(instruction);
			dump_start(dump_address);
			dump_address += 160;
			if (dump_address >= MEMORY_SIZE) dump_address = 0;
		}
		else if(strcmp(token[0], "du") == 0 || strcmp(token[0], "dump") == 0){
			int address_dec_start = -1;
			int address_dec_end = -1;
			// start또는 end와 같이 들어온 dump
			if(end_idx == 2){ // start만 들어왔거나 AA,BB인 형태
				address_dec_start = hexadecimal_to_decimal(token[1]);
				
				if(address_dec_start >= 0){ // start만 들어온게 맞는 경우 (comma 없음)
					if(dump_start(address_dec_start) == -1) // address가 범위 초과일 경우
						printf("Out of Range!\n");
					else{
						create_history_tok(token);
						dump_address = address_dec_start + 160;
						if(dump_address >= MEMORY_SIZE) dump_address = 0;
					}
				}
				else if(strchr(token[1], ',')){ // AA,BB 형태인 경우
					char *tmp_ptr[2];
					tmp_ptr[0] = strtok(token[1], ",");
					address_dec_start = hexadecimal_to_decimal(tmp_ptr[0]);
					tmp_ptr[1] = strtok(NULL, ",");
					address_dec_end = hexadecimal_to_decimal(tmp_ptr[1]);
					strcpy(token[1], tmp_ptr[0]);
					strcpy(token[2], ",\0");
					strcpy(token[3], tmp_ptr[1]);

					if( (address_dec_start >= 0) && (address_dec_end >= 0)){ // 올바른 16진수가 들어온 경우
						if(address_dec_start <= address_dec_end){
							if(dump_start_end(address_dec_start, address_dec_end) != -1){
								create_history_tok(token);
								dump_address = address_dec_end + 1;
								if(dump_address >= MEMORY_SIZE) dump_address = 0;
							}
							else
								printf("Out of Range!\n");
						}
						else
							printf("Please Input Correct Address!\n");

					}
					else
						printf("This is not hexadecimal.\n");

				}
				else
					printf("This is not hexadecimal.\n");
			}
			else if(end_idx == 3){ // start와 end가 같이 들어온 경우
				int comma_flag = 0;
				char tmp_tok[INSTRUCTION_LEN] = {0, };
				if(strchr(token[1], ',')){ // AA, BB
					strncpy(tmp_tok, token[1], strlen(token[1]) -1);
					strcpy(token[1], tmp_tok);
					comma_flag = 1;
				}
				else if(strchr(token[2], ',')) {// AA ,BB
					strcpy(tmp_tok, token[2] + 1);
					strcpy(token[2], tmp_tok);
					comma_flag = 1;
				}
				if(comma_flag){
					address_dec_start = hexadecimal_to_decimal(token[1]);
					address_dec_end = hexadecimal_to_decimal(token[2]);
					if( (address_dec_start >= 0) && (address_dec_end >= 0)){ // 올바른 16진수가 들어온 경우
						if(address_dec_start <= address_dec_end){
							if(dump_start_end(address_dec_start, address_dec_end) != -1){
								create_history_tok(token);
								dump_address = address_dec_end + 1;
								if(dump_address >= MEMORY_SIZE) dump_address = 0;
							}
							else
								printf("Out of Range!\n");
						}
						else
							printf("Please Input Correct Address!\n");
					}
					else
						printf("This is not hexadecimal.\n");
				}
				else
					printf("Please Input Correct Instruction.\n");

			}
			else if(end_idx == 4){ // start , end 꼴로 들어온 경우
				if(strcmp(token[2], ",") == 0){ // start와 end 사이에 콤마가 존재할 때
					address_dec_start = hexadecimal_to_decimal(token[1]);
					address_dec_end = hexadecimal_to_decimal(token[3]);
					if( (address_dec_start >= 0) && (address_dec_end >= 0)){ // 올바른 형태의 16진수
						if(address_dec_start <= address_dec_end){
							if(dump_start_end(address_dec_start, address_dec_end) != -1){
								create_history_tok(token);
								dump_address = address_dec_end + 1;
								if(dump_address >= MEMORY_SIZE) dump_address = 0;
							}
							else
								printf("Out of Range!\n");
						}
						else
							printf("Please Input Correct Address!\n");
					}
					else
						printf("This is not hexadecimal.\n");

				}
				else
					printf("Please Input Correct Instruction.\n");
			}
			// printf("%d %d\n", address_dec_start, address_dec_end); // 10진수 확인
		}
		else if(strcmp(token[0], "e") == 0 || strcmp(token[0], "edit") == 0){
			int edit_address = -1;
			int edit_value = -1;
			if(end_idx == 2){ // address,value 인 형태
				if(strchr(token[1], ',')){ 
					char *tmp_ptr[2];
					tmp_ptr[0] = strtok(token[1], ",");
					edit_address = hexadecimal_to_decimal(tmp_ptr[0]);
					tmp_ptr[1] = strtok(NULL, ",");
					edit_value = hexadecimal_to_decimal(tmp_ptr[1]);
					strcpy(token[1], tmp_ptr[0]);
					strcpy(token[2], ",\0");
					strcpy(token[3], tmp_ptr[1]);

					if( (edit_address >= 0) && (edit_value >= 0)){ // 올바른 16진수가 들어온 경우
						if(edit_address < MEMORY_SIZE && edit_value < 16*16){
							create_history_tok(token);
							edit(edit_address, edit_value);
						}
						else
							printf("Out of Range!\n");
					}
					else
						printf("This is not hexadecimal.\n");
				}
				else
					printf("Please Input Correct Instruction.\n");
			}
			else if(end_idx == 3){ // address, value  OR  address ,value
				int comma_flag = 0;
				char tmp_tok[INSTRUCTION_LEN] = {0, };
				if(strchr(token[1], ',')){ // AA, BB
					strncpy(tmp_tok, token[1], strlen(token[1]) -1);
					strcpy(token[1], tmp_tok);
					comma_flag = 1;
				}
				else if(strchr(token[2], ',')) {// AA ,BB
					strcpy(tmp_tok, token[2] + 1);
					strcpy(token[2], tmp_tok);
					comma_flag = 1;
				}
				if(comma_flag){
					edit_address = hexadecimal_to_decimal(token[1]);
					edit_value = hexadecimal_to_decimal(token[2]);
					if( (edit_address >= 0) && (edit_value >= 0)){ // 올바른 16진수가 들어온 경우
						if(edit_address < MEMORY_SIZE && edit_value < 16*16){
							create_history_tok(token);
							edit(edit_address, edit_value);	
						}
						else
							printf("Out of Range!\n");
					}
					else
						printf("This is not hexadecimal.\n");
				}
				else
					printf("Please Input Correct Instruction.\n");

			}
			else if(end_idx == 4){
				if(strcmp(token[2], ",") == 0){ //  address와 value사이에 콤마가 존재할 때
					edit_address = hexadecimal_to_decimal(token[1]);
					edit_value = hexadecimal_to_decimal(token[3]);
					if( (edit_address >= 0) && (edit_value >= 0)){ // 올바른 형태의 16진수
						if(edit_address < MEMORY_SIZE && edit_value < 16*16){
							create_history_tok(token);
							edit(edit_address, edit_value);
						}
						else
							printf("Out of Range!\n");
					}
					else
						printf("This is not hexadecimal.\n");

				}
				else
					printf("Please Input Correct Instruction.\n");

			}
		}
		else if(strcmp(instruction, "reset") == 0){
			create_history_ins(instruction);
			reset();
		}
		else if(strcmp(token[0], "f") == 0 || strcmp(token[0], "fill") == 0){
			int address_dec_start = -1;
			int address_dec_end = -1;
			int value = -1;
			if(end_idx == 2){ // address,end,value 의 형태
				char *tmp_ptr[3] = {NULL, };
				tmp_ptr[0] = strtok(token[1], ",");
				address_dec_start = hexadecimal_to_decimal(tmp_ptr[0]);
				tmp_ptr[1] = strtok(NULL, ",");
				address_dec_end = hexadecimal_to_decimal(tmp_ptr[1]);
				tmp_ptr[2] = strtok(NULL, ",");
				value = hexadecimal_to_decimal(tmp_ptr[2]);

				strcpy(token[1], tmp_ptr[0]);
				strcpy(token[2], tmp_ptr[1]);
				strcpy(token[3], tmp_ptr[2]);

				if( (address_dec_start >= 0) && (address_dec_end >= 0)){ // 올바른 16진수가 들어온 경우
					if(address_dec_start <= address_dec_end){
						if(fill(address_dec_start, address_dec_end, value) != -1){
							create_history_tok(token);
						}
						else
							printf("Out of Range!\n");
					}
					else
						printf("Please Input Correct Address!\n");

				}
				else
					printf("This is not hexadecimal.\n");
			}
			else if(end_idx >= 3){ // address, end,value 또는 address,end, value 등등
				int comma_flag = 0;
				char *tmp_ptr=NULL;
				char long_token[9*INSTRUCTION_LEN] = {0 ,};
				for(int i=1 ; i<10 ; i++)
					strcat(long_token, token[i]);

				// printf("long_token: %s\n", long_token); // 잘 붙여졌나 확인
				tmp_ptr = strtok(long_token, ",");
				int tok_idx = 1;
				while(1){
					if(tmp_ptr==NULL) break;
					comma_flag = 1;
					strcpy(token[tok_idx], tmp_ptr);
					tmp_ptr = strtok(NULL, ",");
					tok_idx+=2;
					if(tok_idx == 7) break;
				}
				//for(int i=1 ; i<6 ; i+=2)
				//	printf("%s\n",token[i]);

				strcpy(token[2], ",\0");
				strcpy(token[4], ",\0");

				if(comma_flag){
					address_dec_start = hexadecimal_to_decimal(token[1]);
					address_dec_end = hexadecimal_to_decimal(token[3]);
					value = hexadecimal_to_decimal(token[5]);
					if( (address_dec_start >= 0) && (address_dec_end >= 0)){ // 올바른 16진수가 들어온 경우
						if(address_dec_start <= address_dec_end){
							if(fill(address_dec_start, address_dec_end, value) != -1){
								create_history_tok(token);
							}
							else
								printf("Out of Range!\n");
						}
						else
							printf("Please Input Correct Address!\n");
					}
					else
						printf("This is not hexadecimal.\n");
				}
				else
					printf("Please Input Correct Instruction.\n");

			}
		}
		else if(strcmp(token[0], "opcode") == 0){
			// token[1]을 find_hash 함수로 확인
			// 맞으면 해당하는 opcode hashtable에서 찾아서 출력
			// 아니면 실행 하지 않음.
			char opcode = find_hash(token[1]);
			if(opcode != -1){
				printf("opcode is ");
				print_hex_from_dec(opcode);
				printf("\n");
				create_history_tok(token);
			}
			else printf("Please Input Corrcet Opcode.\n");

		}
		else if(strcmp(instruction, "opcodelist") == 0){
			create_history_ins(instruction);
			print_hashtable(); // Hash Table 출력
		}
		else if(strcmp(token[0], "type") == 0){
			// type filename
			if( type(token[1]) != -1) create_history_tok(token);
			else printf("There is no file!\n");
		}
		else if(strcmp(token[0], "assemble") == 0){
			// assemble filename
			int file_len = pass_one(token[1], &asm_length);
			int chk_flag = 0;
			if (file_len == -1) printf("It is not asm file.\n");
			else{
				chk_flag = pass_two(token[1], file_len, asm_length);
				if(chk_flag != -1) create_history_tok(token);
			}
		}
		else if(strcmp(instruction, "symbol") == 0){
			// symbol
			create_history_ins(instruction);
			print_symbol_table();
		}
		else if(strcmp(token[0], "progaddr") == 0){ // loader 또는 run 명령어 수행시 시작하는 주소 지정
			int tmp_address;
			tmp_address = hexadecimal_to_decimal(token[1]);
			if(tmp_address < 0)
				printf("Please Input Correct Hexadecimal.\n");
			else{
				progaddr = tmp_address;
				create_history_tok(token);
			}
		}
		else if(strcmp(token[0], "loader") == 0){
			int file_num=0;
			for (int i=1 ; i<10; i++){
				if(token[i][0]==0){
					file_num = i-1;
					break;
				}
			}
			// printf("file_num: %d\n", file_num);
			linking_loader(token, file_num);
			create_history_tok(token);
		}
		else
			printf("Please Input Correct Instruction.\n");
	}

	free_history();
	free_hash();

	return 0;
}


void replace(char *str, char input, char output){
	for(int i=0;i<strlen(str); i++){
		if(str[i] == input) {
			str[i] = output;
		}
	}
}


void create_history_ins(char *instruction){ // 명령어가 한 어절만 들어왔을 경우
	//hisptr tmp = NULL;
	hisptr newhis = NULL;
	newhis = (hisptr)malloc(sizeof(his));
	newhis->hisrear = NULL;
	strcpy(newhis->ins_token[0], instruction);
	for(int i=1 ; i<6 ; i++)
		strcpy(newhis->ins_token[i], "\0");
	if(rear == NULL){
		rear = newhis;
		front = newhis;
	}
	else{
		rear->hisrear = newhis;
		rear = newhis;
	}
}

void create_history_tok(char token[10][INSTRUCTION_LEN]){ // 명령어가 두 어절 이상인 경우
	//hisptr tmp = NULL;
	hisptr newhis = NULL;
	newhis = (hisptr)malloc(sizeof(his));
	newhis->hisrear = NULL;
	for (int i=0 ; i<6 ; i++){
		strcpy(newhis->ins_token[i], token[i]);
	}
	if(rear == NULL){
		rear = newhis;
		front = newhis;
	}
	else{
		rear->hisrear = newhis;
		rear = newhis;
	}
}


void print_history(){ // print history
	hisptr tmp = NULL;
	int idx = 1;
	for (tmp = front ; tmp != NULL ; tmp = tmp->hisrear){
		printf("%4d",idx++);
		for(int i=0 ; i<6 ; i++){
			if(strcmp(tmp->ins_token[i], ",") == 0)
				printf("%s", tmp->ins_token[i]);
			else{
				if( (i > 0) && (i%2 == 0)){
					if( strcmp(tmp->ins_token[i], "\0") != 0)
						printf("%c", ',');
				}
				printf(" %s", tmp->ins_token[i]);
				
			}
		}
		printf("\n");
	}
}

void free_history(){ // history의 linked list memory free
	hisptr Dlt = NULL;
	while(front != NULL){
		Dlt = front;
		front = front->hisrear;
		free(Dlt);
	}
}
