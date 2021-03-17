#include "20161615.h"

typedef struct _history* hisptr; // History linked list를 만들기 위한 history 포인터

typedef struct _history{ // 명령어 History 구조체
	char ins_token[6][INSTRUCTION_LEN]; // 명령어 토큰은 최대 6개 (fill start, end, value)
	hisptr hisrear; // 다음 History 노드로 이어주는 링크
}his;


void replace(char *str, char input, char output);

void create_history_ins(char *instruction);
void create_history_tok(char token[10][INSTRUCTION_LEN]);
void print_history();
void free_history();


hisptr front = NULL;
hisptr rear = NULL;

int main(){ 
	char instruction[INSTRUCTION_LEN] = {0, };
	char tokenize[INSTRUCTION_LEN] = {0, };

	int dump_address = 0;
	while(1){
		printf("sicsim> ");
		fgets(instruction, INSTRUCTION_LEN, stdin);
		instruction[strlen(instruction)-1]='\0';
		strcpy(tokenize, instruction);
		// printf("%s\n",instruction); // Test Input
		// printf("%d\n", (int)strlen(instruction));

		int end_idx = 0;
		char token[10][INSTRUCTION_LEN] = {0, };
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
			printf("e[dit] address, value\nf[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\n\n");
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
			dump_address += 159;
			if (dump_address >= MEMORY_SIZE) dump_address = 0;
		}
		else if(strcmp(token[0], "du") == 0 || strcmp(token[0], "dump") == 0){
			int address_dec_start = -1;
			int address_dec_end = -1;
			// start또는 end와 같이 들어온 dump
			if(end_idx == 2){ // start만 들어왔거나 AA,BB인 형태
				address_dec_start = hexadecimal_to_decimal(token[1]);
				
				if(address_dec_start >= 0){ // start만 들어온게 맞는 경우 (comma 없음)
					create_history_tok(token);
					dump_start(address_dec_start);
				}
				else if(strchr(token[1], ',')){ // AA,BB 형태인 경우
					char *tmp_ptr = strtok(token[1], ",");
					address_dec_start = hexadecimal_to_decimal(tmp_ptr);
					tmp_ptr = strtok(NULL, ",");
					address_dec_end = hexadecimal_to_decimal(tmp_ptr);

					if( (address_dec_start >= 0) && (address_dec_end >= 0)){ // 올바른 16진수가 들어온 경우
						create_history_tok(token);
						dump_start_end(address_dec_start, address_dec_end);
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
						create_history_tok(token);
						dump_start_end(address_dec_start, address_dec_end);
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
						create_history_tok(token);
						dump_start_end(address_dec_start, address_dec_end);
					}
					else
						printf("This is not hexadecimal.\n");

				}
				else
					printf("Please Input Correct Instruction.\n");
			}
			// printf("%d %d\n", address_dec_start, address_dec_end); // 10진수 확인
		}
		else
			printf("Please Input Correct Instruction.\n");
	}

	free_history();
	return 0;
}


void replace(char *str, char input, char output){
	for(int i=0;i<sizeof(str); i++){
		if(str[i] == input) str[i] = output;
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
