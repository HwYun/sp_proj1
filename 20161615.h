#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define INSTRUCTION_LEN 100
#define MEMORY_SIZE 1048576


/* my_dir.c */
int instruction_dir();



/* my_memory.c */
int my_pow(int a, int b);
int hexadecimal_to_decimal(char *hexadecimal);
void print_hex_from_dec(char decimal);

int dump_start(int start);
int dump_start_end(int start, int end);

int edit(int address, int value);
