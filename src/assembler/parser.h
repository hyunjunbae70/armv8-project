#ifndef PARSER
#define PARSER

#include "assemble.h"
#include "../defs.h"
#include <stdbool.h>
#include "../utils/hashmap.h"

#define MAX_TOKENS_COUNT 6 // Max count of all opcode, all operands
#define INSTRUCTION_COUNT 31

bool is_label(const char *str);

typedef struct {
  parsed_line_type_t line_t;
  char *tokens[MAX_TOKENS_COUNT];
  int length;
  /* represents additional properties */
  union {
    token_mnemonic_t mnemonic; /* for instructions */
  };
} tokenized_line_t;

extern parsed_line_t parse(char* str_input, u32 address, symbol_table_ptr_t table);

extern bool is_bcond_instruction(char* command);

typedef enum {
  POST_INDEX, 
  PRE_INDEX, 
  UNSIGNED_OFFSET, 
  REGISTER_OFFSET, 
  LOAD_LITERAL
} offset_type_t;

#endif // PARSER
