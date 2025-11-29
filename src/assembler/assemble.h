#ifndef ASSEMBLE
#define ASSEMBLE

#include "../defs.h"
#include <stdbool.h>

#define MAX_REG_NUM 31

typedef enum {
  TOKEN_ADD,
  TOKEN_ADDS,
  TOKEN_SUB,
  TOKEN_SUBS,
  TOKEN_CMP,
  TOKEN_CMN,
  TOKEN_NEG,
  TOKEN_NEGS,
  TOKEN_AND,
  TOKEN_ANDS,
  TOKEN_BIC,
  TOKEN_BICS,
  TOKEN_EOR,
  TOKEN_ORR,
  TOKEN_EON,
  TOKEN_ORN,
  TOKEN_TST,
  TOKEN_MOVK,
  TOKEN_MOVN,
  TOKEN_MOVZ,
  TOKEN_MOV,
  TOKEN_MVN,
  TOKEN_MADD,
  TOKEN_MSUB,
  TOKEN_MUL,
  TOKEN_MNEG,
  TOKEN_B,
  TOKEN_B_AL,
  TOKEN_B_EQ,
  TOKEN_B_GQ,
  TOKEN_B_GT,
  TOKEN_B_LE,
  TOKEN_B_LT,
  TOKEN_B_NE,
  TOKEN_BR,
  TOKEN_STR,
  TOKEN_LDR,
  TOKEN_INT
} token_mnemonic_t;

// Instruction types for instruction_IR
typedef enum {
  INSTR_DATA_PROCESSING,
  INSTR_LOAD_STORE,
  INSTR_BRANCH,
} instruction_type_t;

typedef enum {
  OPERAND_REGISTER,
  OPERAND_IMMEDIATE,
  OPERAND_LITERAL_LABEL,
  OPERAND_LITERAL_ADDRESS,
  OPERAND_SIGNED_IMMEDIATE,
  OPERAND_MEMORY_POST_INDEX,
  OPERAND_MEMORY_PRE_INDEX,
  OPERAND_MEMORY_UNSIGNED_OFFSET,
  OPERAND_MEMORY_REGISTER_OFFSET,
  OPERAND_SHIFT
} operand_type_t;

typedef enum { LSL = 0, LSR, ASR, ROR } shift_t;

// Operand types with an identifier
typedef struct {
  operand_type_t type;
  union {
    struct {
      u8 reg_num;
      bool is_64bit;
    } reg;

    u32 immediate;
    i32 s_immediate;
    shift_t shift_type;
    char *literal_label;
    u32 literal_address;

    struct {
      u8 base_reg;
      union {
        i32 offset_imm;
        u8 offset_reg;
      };
    } memory;
  };
} operand_t;

// instruction_IR
/* e.g. ldr x0, my_value */
typedef struct {
  char *mnemonic;
  token_mnemonic_t mnemonic_tok;
  instruction_type_t instr_type;
  int operand_count;
  operand_t operands[5];
  // u32 address;
} instruction_IR_t;

// label_IR
/* e.g. mylabel: */
typedef struct {
  char *name;
} label_IR_t;

// directive_IR
/* e.g. .int 0x3f */
typedef struct {
  i32 value; // Assuming we know it is .int
} directive_IR_t;

// Types of parsed_line as identifier
typedef enum {
  LINE_INSTRUCTION,
  LINE_DIRECTIVE,
  LINE_LABEL,
  SKIP
} parsed_line_type_t;

// Parsed lines output
typedef struct {
  parsed_line_type_t type;
  union {
    instruction_IR_t instr;
    directive_IR_t dir;
    label_IR_t label;
  };
} parsed_line_t;

// Function pointers for instruction assembler
typedef u32 (*instruction_assembler_fun)(instruction_IR_t *, u32);

#define MEMORY_SIZE (1 << 21)

#endif /* ASSEMBLE */
