#include "20161615.h"

int type(char *filename){

	FILE *fp = fopen(filename, "r");
	
	if (fp == NULL) return -1;
	
	char c = fgetc(fp);

	while ( c != EOF ){
		putchar(c);
		c = fgetc(fp);
	}
	printf("\n");

	fclose(fp);
	return 0;
}
