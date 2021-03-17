#include "20161615.h"


void replace(char *str, char input, char output);

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
		}
		else if(strcmp(instruction, "dir") == 0 || strcmp(instruction, "d") == 0){ // dir
			instruction_dir();
		}
		else{
			printf("Please Input Correct Instruction!\n");
		}
	}
	return 0;
}


void replace(char *str, char input, char output){
	for(int i=0;i<sizeof(str); i++){
		if(str[i] == input) str[i] = output;
	}
}





