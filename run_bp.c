#include "20161615.h"


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

}

void execute_opcode(int format, int opcode, int nixbpe, int target_address){
	int* reg_address[10];
	reg_address[0] = &A; reg_address[1] = &X; reg_address[2] = &L;
	reg_address[3] = &B; reg_address[4] = &S; reg_address[5] = &T;
	reg_address[6] = &F; reg_address[7] = NULL;
	reg_address[8] = &PC: reg_address[9] = &SW;

	if (format == 2){
		int *r1 = reg_address[target_address/16];

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
		unsigned char n, i, x, b, p, e;

		n = nixbpe/32;
		i = (nixbpe/16) % 2;
		x = (nixbpe/8) % 2;
		b = (nixbpe/4) % 2;
		p = (nixbpe/2) % 2;
		e = nixbpe % 2;

		if(p==1) target_address = (signed char)target_address + PC;
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
					memory_value += (int)memory_space[location/16][location%16] * my_pow(2, i*8);
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
			memory_wrote(L, location);
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
			else if(A < momory_value) CC = -1;
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


int* get_reg_address(int reg){
	if(reg == 0x00) return &A;
	else if(reg == 0x01) return &X;
	else if(reg == 0x02) return &L;
	else if(reg == 0x03) return &B;
	else if(reg == 0x04) return &S;
	else if(reg == 0x05) return
}
