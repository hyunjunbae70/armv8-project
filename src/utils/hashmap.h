#ifndef HASHMAP
#define HASHMAP
#include <stdio.h>
#include <stdbool.h>
#include "../defs.h"
#include "../assembler/assemble.h"

struct symbol_table_t;
typedef struct symbol_table_t *symbol_table_ptr_t;

symbol_table_ptr_t create_table_ADT( void ); //returns NULL if fail
u32 get_label_address(symbol_table_ptr_t table, char *label); //returns max value possible address if it isn't there
void free_table(symbol_table_ptr_t table); // call to free the space the table occupies

char* put_label(symbol_table_ptr_t table, char *label, u32 address, bool overwrite);
#endif
