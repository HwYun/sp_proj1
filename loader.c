#include "20161615.h"

// object file을 읽어 linking 작업을 수행 후, 가상메모리에 그 결과를 기록.

void linking_loader(char token[][INSTRUCTION_LEN], int file_num){
	// pass1 후 pass2 진행

	int csaddr=progaddr;
	int total_length=0;

	ExHead *extab = (ExHead*)malloc(sizeof(ExHead)*file_num);

	for(int i=0 ; i<file_num ; i++){ // initialize
		clear_str(extab[i].sec_name);
		extab[i].starting_address = 0;
		extab[i].cslth = 0;
		extab[i].next=NULL;
	}

	for(int i=1 ; i<=file_num ; i++) // pass 1
		l_pass_one(token[i], extab, file_num, &csaddr);

	printf("control symbol address length\nsection name\n"); // print load map
	printf("--------------------------------\n");
	for(int i=0 ; i<file_num ; i++){
		printf("%s\t\t%04X\t%04X\n", extab[i].sec_name, extab[i].starting_address, extab[i].cslth);
		total_length += extab[i].cslth;
		for (exptr tmp=extab[i].next ; tmp != NULL ; tmp = tmp->next){
			printf("\t%-s\t%04X\n", tmp->symbol_name, tmp->address);
		}
	}
	printf("--------------------------------\n");
	printf("\t  total length  %04X\n", total_length);
	free_extab(extab, file_num);
}

void l_pass_one(char *filename, ExHead *extab, int file_num, int *csaddr){
	FILE *fp = fopen(filename, "r");
	char objcode[40]={0, };
	char tmp_str[7]={0, };
	int ex_hash_key=0;

	for(int i=0 ; i<strlen(filename); i++)
		ex_hash_key += (int)filename[i];
	ex_hash_key -= 1;
	ex_hash_key %= file_num;

	while(1){
		for(int i=0 ; i<7 ; i++) tmp_str[i] = 0;
		if(fgets(objcode, 40, fp) == NULL) break;
		if(objcode[0] == 'H'){ // header
			strncpy(tmp_str, objcode + 1, 6);// CSNAME
			strcpy(extab[ex_hash_key].sec_name, tmp_str);
			//printf("\n\n\n%s\n\n\n",extab[hash_key].sec_name);
			clear_str(tmp_str);
			//printf("\n\n\n%s\n\n\n",extab[hash_key].sec_name);

			strncpy(tmp_str, objcode + 7, 6); // CSADDR
			*csaddr+=hexadecimal_to_decimal(tmp_str);
			extab[ex_hash_key].starting_address = *csaddr;
			clear_str(tmp_str);

			strncpy(tmp_str, objcode + 13 , 6); // CSLTH
			extab[ex_hash_key].cslth=hexadecimal_to_decimal(tmp_str);
		}
		else if(objcode[0] == 'D'){ // define
			int iter_cnt=(strlen(objcode)-1)/12;
			char symbol_name[7]={0, };
			char symbol_address[7]={0, };
			int sym_add=0;
			for(int i=0 ; i<iter_cnt*2 ; i+=2){
				strncpy(symbol_name, objcode + 1 + i*6, 6); // symbol name
				strncpy(symbol_address, objcode + 7 + i*6, 6); // symbol address
				sym_add=hexadecimal_to_decimal(symbol_address) + *csaddr;
				printf("%s\t%04X\n",symbol_name, sym_add);
				add_node_extab(extab, ex_hash_key, symbol_name, sym_add);
			}
		}
		else if(objcode[0] == 'E') *csaddr+=extab[ex_hash_key].cslth;
	}

	fclose(fp);
}

void clear_str(char *str){
	for(int i=0 ; i<strlen(str); i++)
		str[i] = 0;
}

void add_node_extab(ExHead *extab, int ex_hash_key, char *symbol_name, int address){
	ExNode *new_node = (ExNode*)malloc(sizeof(ExNode));
	clear_str(new_node->symbol_name);
	strcpy(new_node->symbol_name, symbol_name);
	new_node->address = address;
	new_node->next = NULL;

	if(extab[ex_hash_key].next == NULL){
		extab[ex_hash_key].next = new_node;
	}
	else{
		new_node->next = extab[ex_hash_key].next;
		extab[ex_hash_key].next = new_node;
	}
}

void free_extab(ExHead *extab, int file_num){
	exptr del=NULL;
	for(int i=0 ; i<file_num ; i++){
		while(extab[i].next != NULL){
			del = extab[i].next;
			extab[i].next = extab[i].next->next;
			free(del);
		}
	}
	free(extab);
}


