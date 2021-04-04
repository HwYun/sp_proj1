#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define INSTRUCTION_LEN 100
#define MEMORY_SIZE 1048576




void replace(char *str, char input, char output);

/* my_dir.c */
int instruction_dir();



/* my_memory.c */
int my_pow(int a, int b);
int hexadecimal_to_decimal(char *hexadecimal);
void print_hex_from_dec(char decimal);

int dump_start(int start);
int dump_start_end(int start, int end);
int edit(int address, unsigned char value);
int fill(int start, int end, unsigned char value);
void reset();


/* opcode.c */

typedef struct HashNode* hashptr;

typedef struct HashNode{
	char mnemonic[10];
	char opcode;
	hashptr next;
}HashNode;

void print_hashtable();
int hash_key(char *mnemonic);
void create_opcode_hash();
void add_hash(HashNode* node);
char find_hash(char *mnemonic);
void free_hash();


/* type.c */
int type(char *filename);




