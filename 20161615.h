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
unsigned char memory_space[65536][16];

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

/* proj3 */
int progaddr;


/* loader.c */
typedef struct ExNode* exptr;

typedef struct ExNode{
	char symbol_name[7];
	int address;
	exptr next;
}ExNode;

typedef struct ExHead{
	char sec_name[7];
	int starting_address;
	int cslth;
	exptr next;
}ExHead;

void linking_loader(char token[][INSTRUCTION_LEN], int file_num);
void l_pass_one(char *filename, ExHead *extab, int file_num, int *csaddr);
void l_pass_two(char *filename, ExHead *extab, int file_num, int *csaddr);
int ex_hash_key(char *str, int file_num);
void clear_str(char *str);
void eliminate_space(char *str);
int twos_complement(int value);
void add_node_extab(ExHead *extab, int ex_hash_key, char *symbol_name, int address);
void free_extab(ExHead *extab, int file_num);
