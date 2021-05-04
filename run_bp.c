#include "20161615.h"

int is_continue = 0;

void bp(char *breakpoint){
	// bp [address]
	break_point[bp_num]=hexadecimal_to_decimal(breakpoint);
	bp_num++;
	bp_sort();
	printf("\t\t[ok] create breakpoint %s\n", breakpoint);
}

void bp_clear(){
	for(int i=0 ; i<100 ; i++) break_point[i] = 0;
	bp_num = 0;
	printf("\t\t[ok] clear all breakpoints\n");
}

void print_bp(){
	printf("\t\tbreakpoint\n");
	printf("\t\t----------\n");
	for(int i=0 ; i<bp_num ; i++){
		printf("\t\t%X\n", break_point[i]);
	}
}

void bp_sort(){
	for(int i=0 ; i<bp_num ; i++){
		for(int j=i ; j<bp_num ; j++){
			if(break_point[i] > break_point[j]){
				int tmp = 0;
				tmp = break_point[i];
				break_point[i] = break_point[j];
				break_point[j] = tmp;
			}
		}
	}
}


void my_run(){
	PC = exeaddr;
	int opcode, format, target_address, nixbpe;
	int cnt = 0;
	int max_cnt = prog_len;
	int start_address = progaddr;
	int end_address;
	unsigned char ni;
	end_address = progaddr + max_cnt;
	// printf("%06X // %06X\n", start_address, end_address);
	int is_end = 1;
	int break_flag=0;


	if(!is_continue)
		L = end_address;
	while(1){
		if(PC<start_address || PC>=end_address) break;
		opcode = memory_space[PC/16][PC%16];
		// ...ing
		ni = opcode & (unsigned char)0x03;
		opcode = opcode & (unsigned char)0xFC;
		// printf("opcode: %06X\n", opcode);

		format = opcode_format(opcode);
		// printf("PC: %06X\n", PC);

		if(format == 0){ // opcode가 아닌 경우. 즉 WORD, BYTE 상수
			PC += 3;
		}
		else{
			PC++;
			if(format == 2){
				target_address = (int)memory_space[PC/16][PC%16];
				PC++;
				execute_opcode(format, opcode, 0, target_address);
			}
			else if(format == 3){
				nixbpe = memory_space[PC/16][PC%16] & (unsigned char)0xF0;
				nixbpe = nixbpe >> 4;
				nixbpe += ni << 4;
				target_address = memory_space[PC/16][PC%16] & (unsigned char)0x0F;
				PC++;
				if(nixbpe % 2){ // format 4
					target_address = target_address << 8;
					target_address += memory_space[PC/16][PC%16];
					PC++;
				}
				target_address = target_address << 8;
				target_address += memory_space[PC/16][PC%16];
				PC++;
				execute_opcode(format, opcode, nixbpe, target_address);
			}
		}
		//printf("target: %06X // nixbpe: %02X\nformat: %d  // PC: %06X\n",target_address, nixbpe, format, PC);

		for(int i=0 ; i<bp_num ; i++){
			if(break_point[i] == PC){
				is_end = 0;
				print_register();
				printf("\t\tStop at checkpoint[%X]\n", PC);
				exeaddr=PC;
				break_flag = 1;
				is_continue = 1;
				break;
			}
		}
		if(break_flag) break;

		if(++cnt == max_cnt){
			printf("Error!!\n");
			break;
		}
	}
	if(is_end == 1){
		print_register();
		printf("\t\tEnd Program\n");
		is_continue = 0;
		exeaddr = progaddr;
	}
	
}

void execute_opcode(int format, int opcode, int nixbpe, int target_address){

	if (format == 2){
		int *r1 = reg_address(target_address/16);

		if(opcode == 0xB4){ // CLEAR r1
			*r1 = 0;
		}
		else if(opcode == 0xA0){ // COMPR r1, r2
			// 원래는 r1과 r2 비교하여 CC에 결과를 저장하는 명령어이지만
			// 이번 프로젝트에서는 CC값이 0으로 고정된다. (입력값이 없음)
			CC = 0;
		}
		else if(opcode == 0xB8){ // TIXR r1
			X++;
			if(X > *r1) CC = 0;
			else if(X < *r1) CC = -1;
			else CC = 0;
		}

	}
	else{
		unsigned char n, i, x, b, p;

		n = nixbpe/32;
		i = (nixbpe/16) % 2;
		x = (nixbpe/8) % 2;
		b = (nixbpe/4) % 2;
		p = (nixbpe/2) % 2;

		if(p==1) target_address = (signed char) target_address + PC;
		else if(b==1) target_address += B;

		if(x==1) target_address += X;
		
		int memory_value = 0;
		int location;
		location = target_address;

		if(n == 0 && i == 1){ // immediate 
			memory_value = target_address;
		}
		else if(n == 1 && i == 0){ // indirect
			for (int i=0 ; i<2 ; i++){
				memory_value = 0;
				for(int j=2 ; j>-1 ; j--){
					memory_value += (int)memory_space[location/16][location%16] << (j*8);
					location++;
				}
				if(i==0){
					location = memory_value;
					target_address = memory_value;
				}
			}
		}
		else if(n==1 && i==1){ // simple
			for(int i=2 ; i>-1 ; i--){
				memory_value += (int)memory_space[location/16][location%16] * my_pow(2, i*8);
				location++;
			}
		}


		if(opcode == 0x14){ // STL m
			location -= 3;
			memory_write(L, location);
		}
		else if(opcode == 0x68){ // LDB m
			B = memory_value;
		}
		else if(opcode == 0x48){ // JSUB
			L = PC;
			PC = target_address;
		}
		else if(opcode == 0x00){ // LDA m
			A = memory_value;
		}
		else if(opcode == 0x28){ // COMP m
			if (A > memory_value) CC = 1;
			else if(A < memory_value) CC = -1;
			else CC = 0;
		}
		else if(opcode == 0x30){ // JEQ m
			if(CC == 0) PC = target_address;
		}
		else if(opcode == 0x3C){ // J m
			PC = target_address;
		}
		else if(opcode == 0x0C){ // STA m
			location -= 3;
			memory_write(A, location);
		}
		else if(opcode == 0x74){ // LDT m
			T = memory_value;
		}
		else if(opcode == 0xE0){ // TD m
			CC = -1;
		}
		else if(opcode == 0xD8){ // RD m
			CC = 0;
		}
		else if(opcode == 0x54){ // STCH m
			location -= 3;
			memory_value = A & (unsigned char)0x0FF;
			char tmp_obj[3] = {0, };
			sprintf(tmp_obj, "%X", memory_value);
			memory_space[location/16][location%16] = (unsigned char)hexadecimal_to_decimal(tmp_obj);
		}
		else if(opcode == 0x38){ // JLT m
			if(CC == -1) PC = target_address;
		}
		else if(opcode == 0x10){ // STX m
			location -= 3;
			memory_write(X, location);
		}
		else if(opcode == 0x4C){ // RSUB
			PC = L;
		}
		else if(opcode == 0x50){ // LDCH m
			memory_value = (unsigned)(memory_value >> 16);
			A = (A >> 8) << 8;
			A = A + memory_value;
		}
	}
}

void memory_write(int value, int location){
	char tmp_obj[3] = {0, };
	char tmp_str[7] = {0, };
	int tmp_address;
	tmp_address = location;

	sprintf(tmp_str, "%06X", value);
	for(int i=0 ; i<6 ; i+=2){
		strncpy(tmp_obj, tmp_str + i, 2);
		memory_space[tmp_address/16][tmp_address%16] = (unsigned char)hexadecimal_to_decimal(tmp_obj);
		tmp_address++;
	}

}


void print_register(){
	printf("A : %06X  X : %06X\n", A, X);
	printf("L : %06X PC : %06X\n", L, PC);
	printf("B : %06X  S : %06X\n", B, S);
	printf("T : %06X\n", T);
}

int *reg_address(int num){
	switch (num) {
		case 0:
			return &A;
		case 1:
			return &X;
		case 2:
			return &L;
		case 3:
			return &B;
		case 4:
			return &S;
		case 5:
			return &T;
		case 6:
			return &F;
		case 8:
			return &PC;
		case 9:
			return &SW;
		default:
			return NULL;
	}

}
