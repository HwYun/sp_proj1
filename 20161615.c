#include "20161615.h"

typedef struct _history* hisptr;

typedef struct _history{ // 명령어 History 구조체
	char ins_token[6][INSTRUCTION_LEN]; // 명령어 토큰은 최대 6개
	hisptr hisrear; // 다음 History 노드로 이어주는 링크
}his;


void replace(char *str, char input, char output);

void create_history_ins(char *instruction);
void create_history_tok(char **token);
void print_history();
void free_history();


hisptr front = NULL;
hisptr rear = NULL;

int main(){ 
	char instruction[INSTRUCTION_LEN] = {0, };
	char tokenize[INSTRUCTION_LEN] = {0, };

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
		else{
			printf("Please Input Correct Instruction!\n");
		}
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

void create_history_tok(char **token){ // 명령어가 두 어절 이상인 경우
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
	int idx = 0;
	for (tmp = front ; tmp != NULL ; tmp = tmp->hisrear){
		printf("%4d",idx++);
		for(int i=0 ; i<6 ; i++){
			if(strcmp(tmp->ins_token[i], ",") == 0)
				printf("%s", tmp->ins_token[i]);
			else
				printf(" %s", tmp->ins_token[i]);
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
