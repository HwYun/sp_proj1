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

	csaddr = progaddr;
	for(int i=1 ; i<=file_num ; i++)
		l_pass_two(token[i], extab, file_num, &csaddr);

	printf("control symbol address length\nsection name\n"); // print load map
	printf("--------------------------------\n");
	for(int i=0 ; i<file_num ; i++){
		printf("%s\t\t%04X\t%04X\n", extab[i].sec_name, extab[i].starting_address, extab[i].cslth);
		total_length += extab[i].cslth;
		for (exptr tmp=extab[i].next ; tmp != NULL ; tmp = tmp->next){
			printf("\t%6s\t%04X\n", tmp->symbol_name, tmp->address);
		}
	}
	printf("--------------------------------\n");
	printf("\t  total length  %04X\n", total_length);
	free_extab(extab, file_num);
}

void l_pass_one(char *filename, ExHead *extab, int file_num, int *csaddr){
	FILE *fp = fopen(filename, "r");
	char objcode[100]={0, };
	char tmp_str[7]={0, };
	char control_sec[7] = {0, };
	int ex_key=0;


	while(1){
		for(int i=0 ; i<7 ; i++) tmp_str[i] = 0;
		if(fgets(objcode, 100, fp) == NULL) break;
		if(objcode[0] == 'H'){ // header
			strncpy(control_sec, objcode + 1, 6);// CSNAME
			eliminate_space(control_sec);
			ex_key = ex_hash_key(control_sec, file_num);
			strcpy(extab[ex_key].sec_name, control_sec);

			strncpy(tmp_str, objcode + 7, 6); // CSADDR
			*csaddr+=hexadecimal_to_decimal(tmp_str);
			extab[ex_key].starting_address = *csaddr;
			clear_str(tmp_str);

			strncpy(tmp_str, objcode + 13 , 6); // CSLTH
			extab[ex_key].cslth=hexadecimal_to_decimal(tmp_str);
			clear_str(tmp_str);
		}
		else if(objcode[0] == 'D'){ // define
			int iter_cnt=(strlen(objcode)-1)/12;
			char symbol_name[7]={0, };
			char symbol_address[7]={0, };
			int sym_add=0;
			for(int i=0 ; i<iter_cnt*2 ; i+=2){
				strncpy(symbol_name, objcode + 1 + i*6, 6); // symbol name
				eliminate_space(symbol_name);
				ex_key = ex_hash_key(symbol_name, file_num);
				strncpy(symbol_address, objcode + 7 + i*6, 6); // symbol address
				sym_add=hexadecimal_to_decimal(symbol_address) + *csaddr;
				// printf("%s\t%04X\n",symbol_name, sym_add);
				add_node_extab(extab, ex_key, symbol_name, sym_add);
			}
		}
		else if(objcode[0] == 'E') {
			ex_key = ex_hash_key(control_sec, file_num);
			*csaddr+=extab[ex_key].cslth;
		}
	}

	fclose(fp);
}

void l_pass_two(char *filename, ExHead *extab, int file_num, int *csaddr){
	FILE *fp = fopen(filename, "r");
	char objcode[100] = {0, };
	char tmp_str[7] = {0, };
	char control_sec[7] = {0, };
	char ref_num[10][7] = {{0, },};
	int cur_address = 0;
	int ex_key = 0;
	int cslth=0;

	while(1){
		clear_str(objcode);
		clear_str(tmp_str);
		if(fgets(objcode, 100, fp) == NULL) break;
		if(objcode[0] == 'H'){ // header
			strncpy(control_sec, objcode + 1, 6);// CSNAME
			eliminate_space(control_sec);
			ex_key = ex_hash_key(control_sec, file_num);
			strcpy(ref_num[1], control_sec);

			strncpy(tmp_str, objcode + 7, 6); // CSADDR
			*csaddr+=hexadecimal_to_decimal(tmp_str);
			clear_str(tmp_str);
			
			strncpy(tmp_str, objcode + 13 , 6); // CSLTH
			cslth=hexadecimal_to_decimal(tmp_str);

		}
		else if(objcode[0] == 'R'){
			int tmp_iter = (strlen(objcode)-1)/8 + 1;
			int tmp_idx = 0;

			// for(int i=0 ; i<tmp_iter ; i++) clear_str(ref_num[i]);
			
			for(int i=0 ; i<tmp_iter ; i++){
				char tmp_num[3] = {0, };
				strncpy(tmp_num, objcode + 1 + i*8, 2); // ref_num
				tmp_idx = hexadecimal_to_decimal(tmp_num);
				//printf("ref_num: %s ; ", tmp_num);
				strncpy(tmp_str, objcode + 3 + i*8, 6); // symbol
				eliminate_space(tmp_str);
				//printf("symbol: %s\n", tmp_str);
				strcpy(ref_num[tmp_idx], tmp_str);
				clear_str(tmp_str);
			}

			for(int i=1 ; i<=tmp_iter+1 ; i++){
				printf("%02d: %s\n", i, ref_num[i]);
			}
		}
		else if(objcode[0] == 'T'){
			char starting_add[7]={0, };
			char objcode_len[3] = {0, };
			int obj_len = 0;
			strncpy(starting_add, objcode + 1, 6); // address
			cur_address = hexadecimal_to_decimal(starting_add) + *csaddr;
			printf("csaddr: %04X // startind_add: %s\n", *csaddr, starting_add);
			strncpy(objcode_len, objcode + 7, 2); // objcode len
			obj_len = hexadecimal_to_decimal(objcode_len);
			printf("obj_len: %d\n",obj_len);
			char *tmp_add=objcode + 9;
			char tmp_obj[3] = {0, };
			for(int i=0; i<obj_len*2 ; i+=2){
				strncpy(tmp_obj, tmp_add + i, 2);
				memory_space[cur_address/16][cur_address%16]=(unsigned char)hexadecimal_to_decimal(tmp_obj);
				cur_address++;
			}
		
		}
		else if(objcode[0] == 'M'){
			// Modification
		}
		else if(objcode[0] == 'E')
			*csaddr += cslth;

	}

	fclose(fp);
	

}

int ex_hash_key(char *str, int file_num){
	int key = (int)str[strlen(str)-1] + 1;
	return key%file_num;
}

void clear_str(char *str){
	for(int i=0 ; i<strlen(str); i++)
		str[i] = 0;
}

void eliminate_space(char *str){
	for(; *str != '\0' ; str++){
		if(*str==' '){
			strcpy(str, str+1);
			str--;
		}
	}
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


