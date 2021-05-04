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
	prog_len = total_length;
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
	int ref_num[10] = {0, };
	int cur_address = 0;
	int ex_key = 0;
	int cslth=0;
	int cur_mod=-1;
	int calc_value = 0;
	int byte_len = 0;

	while(1){
		clear_str(objcode);
		clear_str(tmp_str);
		if(fgets(objcode, 100, fp) == NULL) break;
		if(objcode[0] == 'H'){ // header
			strncpy(control_sec, objcode + 1, 6);// CSNAME
			eliminate_space(control_sec);
			ex_key = ex_hash_key(control_sec, file_num);
			ref_num[1] = extab[ex_key].starting_address;

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
				for(int i=0 ; i<strlen(tmp_str) ; i++){
					if(tmp_str[i] == '\n'){
						tmp_str[i] = '\0';
						break;
					}
				}
				//printf("symbol: %s\n", tmp_str);
				//strcpy(ref_num[tmp_idx], tmp_str);
				ex_key = ex_hash_key(tmp_str, file_num);
				for(exptr tmp_ptr = extab[ex_key].next ; tmp_ptr != NULL ; tmp_ptr = tmp_ptr->next){
					// printf("%s :: %s\n", tmp_ptr->symbol_name, tmp_str);
					if(strcmp(tmp_ptr->symbol_name, tmp_str) == 0){
						ref_num[tmp_idx] = tmp_ptr->address;
					}
				}
				// printf("%s / %04X\n", tmp_str, ref_num[tmp_idx]);
				clear_str(tmp_str);
			}

			/*
			for(int i=1 ; i<=tmp_iter+1 ; i++){
				printf("%02d: %s\n", i, ref_num[i]);
			}
			*/
		}
		else if(objcode[0] == 'T'){
			char starting_add[7]={0, };
			char objcode_len[3] = {0, };
			int obj_len = 0;
			strncpy(starting_add, objcode + 1, 6); // address
			cur_address = hexadecimal_to_decimal(starting_add) + *csaddr;
			// printf("csaddr: %04X // startind_add: %s\n", *csaddr, starting_add);
			strncpy(objcode_len, objcode + 7, 2); // objcode len
			obj_len = hexadecimal_to_decimal(objcode_len);
			// printf("obj_len: %d\n",obj_len);
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
			char mod_address[7] = {0, };
			char mod_byte[3] = {0, };
			char mod_sign[2] = {0, };
			char reference_num[3] = {0, };
			int ref_idx=0;

			strncpy(mod_address, objcode + 1, 6);
			strncpy(mod_byte, objcode + 7, 2);
			strncpy(mod_sign, objcode + 9, 1);
			strncpy(reference_num, objcode + 10, 2);
			ref_idx = atoi(reference_num);

			// printf("%s\n%s^%s^%s%02X\n", ref_num[1], mod_address, mod_byte, mod_sign, ref_idx);
			

			if(cur_mod == hexadecimal_to_decimal(mod_address)){ // 이전 modification에 이어서 계산
				if(mod_sign[0] == '+') calc_value += ref_num[ref_idx];
				else if(mod_sign[0] == '-') calc_value -= ref_num[ref_idx];
			}
			else if(cur_mod == -1){ // 처음 계산만 한다.
				calc_value = 0;
				cur_address = hexadecimal_to_decimal(mod_address) + *csaddr;
				cur_mod = cur_address - *csaddr;
				byte_len = hexadecimal_to_decimal(mod_byte) + 1;
				byte_len /=2;

				for(int i=byte_len-1 ; i>=0; i--){
					calc_value+=(int)memory_space[cur_address/16][cur_address%16] * my_pow(256,i);
					cur_address++;
				}
				// printf("초기값: %06X\n", calc_value);

				if(calc_value >= my_pow(2, 23)){ // 음수일 때
					calc_value = twos_complement(calc_value);
				}

				if(mod_sign[0] == '+') calc_value += ref_num[ref_idx];
				else if(mod_sign[0] == '-')	calc_value -= ref_num[ref_idx];
			}
			else{ // 메모리에 저장하고 현재 modification 계산 실행
				char mod_value[7];

				sprintf(mod_value, "%06X", calc_value);
				// printf("value: %s\n",mod_value);
				cur_address = cur_mod + *csaddr;
				char tmp_obj[3] = {0, };
				for(int i=0 ; i<byte_len*2 ; i+=2){
					strncpy(tmp_obj, mod_value + i, 2);
					memory_space[cur_address/16][cur_address%16]=(unsigned char)hexadecimal_to_decimal(tmp_obj);
					cur_address++;
				}

				cur_address = hexadecimal_to_decimal(mod_address) + *csaddr;
				cur_mod = cur_address - *csaddr;

				byte_len = hexadecimal_to_decimal(mod_byte) + 1;
				byte_len /=2;

				calc_value = 0;
				for(int i=byte_len-1 ; i>=0; i--){
					calc_value+=(int)memory_space[cur_address/16][cur_address%16] * my_pow(256,i);
					cur_address++;
				}
				// printf("초기값: %06X\n", calc_value);
				if(calc_value>=my_pow(2,23)){ // 음수일 때.
					calc_value = twos_complement(calc_value);
				}
				if(mod_sign[0] == '+') calc_value += ref_num[ref_idx];
				else if(mod_sign[0] == '-')	calc_value -= ref_num[ref_idx];

			}
			//printf("%06X\n", cur_mod);

			
		}
		else if(objcode[0] == 'E'){
			if(cur_mod != -1){
				char mod_value[7] = {0, };
				sprintf(mod_value, "%06X", calc_value);
				cur_address = cur_mod + *csaddr;
				char tmp_obj[3] = {0, };
				for(int i=0 ; i<byte_len*2 ; i+=2){
					strncpy(tmp_obj, mod_value + i, 2);
					memory_space[cur_address/16][cur_address%16]=(unsigned char)hexadecimal_to_decimal(tmp_obj);
				cur_address++;
				}
			}
			*csaddr += cslth;
		}

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

int twos_complement(int value){
	value = value|0b11111111000000000000000000000000;
	value = ~value;
	// printf("t_c: %d\n", value + 1);
	return (value + 1) * (-1);
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


