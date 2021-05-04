#include "20161615.h"


void print_hashtable(){
	int head_chk=0;
	for (int i=0 ; i<20 ; i++){
		HashNode* tmp = hash_table[i];
		printf("%d :", i);
		head_chk = 1;
		while(tmp != NULL){
			if(!head_chk){ // 처음 노드이면
				printf(" -> ");
			}
			else // 첫 노드가 아니면
				head_chk = 0;
			printf("[%s,%2X]", tmp->mnemonic, tmp->opcode);
			// print_hex_from_dec(tmp->opcode);
			// printf("\b]");
			tmp = tmp->next;
		}
		printf("\n");
	}

}

int hash_key(char *mnemonic){
	int result = 0;
	for(int i=0 ; i<strlen(mnemonic) ; i++)
		result += (int)mnemonic[i];
	return result%20;
}

void create_opcode_hash(){
	FILE *fp=fopen("opcode.txt","r");
	char op_line[INSTRUCTION_LEN];
	char *tok_ptr;
	int code;
	char mnem[10];
	char form[5];

	while(1){
		for(int i=0 ; i<INSTRUCTION_LEN ; i++) op_line[i] = 0;
		if(fgets(op_line, INSTRUCTION_LEN, fp) == NULL) break;
		char tmp_op_line[INSTRUCTION_LEN]; // tokenize를 위한 tmp line
		op_line[strlen(op_line)-1] = '\0';
		replace(op_line, 9, 32);
		replace(op_line, 13, 32);
		//for(int i=0;i<strlen(op_line);i++) printf("%d ",op_line[i]);
		strcpy(tmp_op_line, op_line);
		tok_ptr = strtok(tmp_op_line, " ");
		code = hexadecimal_to_decimal(tok_ptr);
		tok_ptr = strtok(NULL, " ");
		strcpy(mnem, tok_ptr);
		tok_ptr = strtok(NULL, " ");
		strcpy(form, tok_ptr);

		eliminate_space(form);
		
		HashNode* node = (HashNode*)malloc(sizeof(HashNode));
		strcpy(node->mnemonic, mnem);
		strcpy(node->format, form);
		node->opcode = code;
		// printf("mnemonic: %s // format: %s // opcode: %04X\n", node->mnemonic, node->format, node->opcode); // 잘 작동하나 확인
		node->next = NULL;
		add_hash(node);
	}
	fclose(fp);
}

void add_hash(HashNode* node){
	int key = hash_key(node->mnemonic);
	if(hash_table[key] == NULL){
		hash_table[key] = node;
	}
	else{
		node->next = hash_table[key];
		hash_table[key] = node;
	}
	// printf("%s\n", node->mnemonic);
}

int find_hash(char *mnemonic){
	int key = hash_key(mnemonic);
	HashNode* tmp = NULL;
	for (tmp = hash_table[key] ; tmp != NULL; tmp = tmp->next){
		if(strcmp(tmp->mnemonic, mnemonic) == 0){
			return tmp->opcode;
		}
	}
	return -1; // 아무것도 못찾았을 경우
}

int find_format(char *mnemonic, char *format){
	int key = hash_key(mnemonic);
	HashNode* tmp = NULL;
	for (tmp = hash_table[key] ; tmp != NULL; tmp = tmp->next){
		if(strcmp(tmp->mnemonic, mnemonic) == 0){
			strcpy(format, tmp->format);
			return 1;
		}
	}
	return -1; // 아무것도 못찾았을 경우
}

void free_hash(){
	HashNode* del = NULL;
	for(int i=0 ; i<20 ; i++){
		while(hash_table[i] != NULL){
			del = hash_table[i];
			hash_table[i] = hash_table[i]->next;
			free(del);
		}
	}
}


int opcode_format(int opcode){
	// printf("opcode format에서 찾는 중!! %2X\n", opcode);
	HashNode *tmp = NULL;
	for(int i=0 ; i<20 ; i++){
		for(tmp = hash_table[i] ; tmp!=NULL; tmp = tmp->next)
			if(tmp->opcode == opcode){
				// printf("%2X // %s\n", opcode, tmp->format);
				if(strcmp(tmp->format, "3/4\0") == 0) return 3;
				else if(strcmp(tmp->format, "2\0") == 0) return 2;
				else return 0;
			}
	}
	return 0;
}
