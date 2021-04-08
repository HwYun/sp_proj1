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
	char format[5];
	hashptr next;
}HashNode;

HashNode *hash_table[20];

void print_hashtable();
int hash_key(char *mnemonic);
void create_opcode_hash();
void add_hash(HashNode* node);
char find_hash(char *mnemonic);
int find_format(char *mnemonic, char *format);
void free_hash();


/* type.c */
int type(char *filename);


/* assemble.c */

typedef struct _symbol{
	char symbol[10];
	int location;
}Symbol;

Symbol symbol_tab[100];
Symbol last_symbol_tab[100];

typedef struct source_statement{
	int loc;
	char label[20];
	char opcode[20];
	char operand[20];
	char obj_code[10];
	char comment[100];
}SrcState;

int find_sym(char *label);
int pass_one(char *filename, int *length);
int pass_two(char *filename, int file_line_len, int length);
void generate_obj_lst(SrcState *stat, char *filename, int length);
void error_handling(int line_num);
void print_symbol_table();
