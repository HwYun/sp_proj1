#include "20161615.h"

unsigned char memory_space[65536][16];


int my_pow(int a, int b){
	int result = 1;
	if(b==0) return 1;
	else{
		for (int i=0 ; i < b ; i++)
			result *= a;
	}
	return result;
}

int hexadecimal_to_decimal(char *hexadecimal){ // 16진수 문자열을 10진수 정수로 바꾸는 함수
	int decimal = 0;
	int pos = 0;

	for (int i = strlen(hexadecimal) - 1 ; i>=0 ; i--){
		char ch = hexadecimal[i];
		if ((ch >= '0') && (ch <= '9')) // 0~9
			decimal += (ch - '0') * my_pow(16, pos);
		else if ((ch >= 'A') && (ch <= 'F')) //  A~F
			decimal += (ch - 55) * my_pow(16, pos);
		else if ((ch >= 'a') && (ch <= 'f')) // a~f
			decimal += (ch - 87) * my_pow(16, pos);
		else
			return -1; // 16진수가 아님
		pos++;
	}
	return decimal;
}


// 10진수를 16진수로 바꾸어 출력해주는 함수 (1byte의 데이터만 가능)
void print_hex_from_dec(char decimal){
	char hex[3] = {0, };

	unsigned int pos = 0;
	for( pos = 0 ; pos < 2 ; pos++){
		char mod = decimal % 16;
		if (mod < 10)
			hex[pos] = '0' + mod;
		else
			hex[pos] = 'A' - 10 + mod;
		
		decimal /= 16;

	}
	for(int i = pos - 1; i >= 0; i--)
		printf("%c", hex[i]);
	printf(" ");
}


int dump_start(int start){ // start 주소에서부터 출력.
	int end = start + 159;
	if(end >= MEMORY_SIZE) end = MEMORY_SIZE -1;
	return dump_start_end(start, end);	
}

int dump_start_end(int start, int end){ // start에서 end까지 출력.
	int start_add = start;
	int end_add = end;
	if(start_add < 0 || start_add >= MEMORY_SIZE) return -1;
	if(end_add < 0 || end_add >= MEMORY_SIZE) return -1;

	int start_line = start_add / 16;
	int end_line = end_add / 16;
	char address[6] = {0, };
	int print_chk = start_line * 16;

	for(int i = start_line ; i <= end_line ; i++){
		int i_address = i;
		int hex_div = my_pow(16, 3);
		for( int j = 0 ; j < 4 ; j++){
			if( i_address/hex_div <= 9) address[j] = (char)(i_address/hex_div + '0');
			else
				address[j] = (char)(i_address/hex_div - 10 + 'A');
			i_address = i_address % hex_div;
			hex_div /= 16;
		}
		address[4] = '0';
		printf("%s ",address);

		for (int j = 0;j < 16;j++){
			if((print_chk < start_add) || (print_chk > end_add)){
				printf("   ");
			}
			else{
				print_hex_from_dec(memory_space[i][j]);
			}
			print_chk++;
		}
		printf("; ");
		print_chk -=16;
		for( int j = 0 ; j < 16 ; j++){
			if((print_chk < start_add) || (print_chk > end_add)){
				printf(".");
			}
			else{
				if((int)(memory_space[i][j]) < 32 || (int)memory_space[i][j] > 126){
					printf(".");
				}
				else{
					printf("%c", memory_space[i][j]);
				}
			}
			print_chk++;
		}
		printf("\n");
	}

	return 0;
}
