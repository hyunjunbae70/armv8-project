#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "parser.h"
#include "assemble.h"
#include "../utils/hashmap.h"

#define is_immediate(x) (x[0] == '#')

typedef struct {
  const char *mnemonic;
  token_mnemonic_t token;
} mnemonic_mapping_t;

static const mnemonic_mapping_t mnemonic_table[] = {
    {"add", TOKEN_ADD},   {"adds", TOKEN_ADDS}, {"and", TOKEN_AND},
    {"ands", TOKEN_ANDS}, {"b", TOKEN_B},       {"b.al", TOKEN_B_AL},
    {"b.eq", TOKEN_B_EQ}, {"b.ge", TOKEN_B_GQ}, {"b.gt", TOKEN_B_GT},
    {"b.le", TOKEN_B_LE}, {"b.lt", TOKEN_B_LT}, {"b.ne", TOKEN_B_NE},
    {"bic", TOKEN_BIC},   {"bics", TOKEN_BICS}, {"br", TOKEN_BR},
    {"cmn", TOKEN_CMN},   {"cmp", TOKEN_CMP},   {"eon", TOKEN_EON},
    {"eor", TOKEN_EOR},   {"ldr", TOKEN_LDR},   {"madd", TOKEN_MADD},
    {"mneg", TOKEN_MNEG}, {"mov", TOKEN_MOV},   {"movk", TOKEN_MOVK},
    {"movn", TOKEN_MOVN}, {"movz", TOKEN_MOVZ}, {"msub", TOKEN_MSUB},
    {"mul", TOKEN_MUL},   {"mvn", TOKEN_MVN},   {"neg", TOKEN_NEG},
    {"negs", TOKEN_NEGS}, {"orn", TOKEN_ORN},   {"orr", TOKEN_ORR},
    {"str", TOKEN_STR},   {"sub", TOKEN_SUB},   {"subs", TOKEN_SUBS},
    {"tst", TOKEN_TST},   {".int", TOKEN_INT}};

#define MNEMONIC_TABLE_SIZE (sizeof(mnemonic_table) / sizeof(mnemonic_table[0]))

// Comparison function for bsearch
static int compare_mnemonic_mapping(const void *a, const void *b) {
  const char *key = (const char *)a;
  const mnemonic_mapping_t *mapping = (const mnemonic_mapping_t *)b;
  return strcmp(key, mapping->mnemonic);
}

static token_mnemonic_t get_mnemonic_token(const char *mnemonic) {
  mnemonic_mapping_t *result =
      bsearch(mnemonic, mnemonic_table, MNEMONIC_TABLE_SIZE,
              sizeof(mnemonic_mapping_t), compare_mnemonic_mapping);

  if (result) {
    return result->token;
  } else {
    fprintf(stderr, "Unknown mnemonic: %s\n", mnemonic);
    exit(1);
  }
}

#define NR_INSTRUCTIONS 37
static const char *ALL_INSTRUCTIONS[NR_INSTRUCTIONS] = {
    "add",  "adds", "and",  "ands", "b",    "b.al", "b.eq", "b.ge",
    "b.gt", "b.le", "b.lt", "b.ne", "bic",  "bics", "br",   "cmn",
    "cmp",  "eon",  "eor",  "ldr",  "madd", "mov",  "movk", "movn",
    "movz", "mneg", "msub", "mul",  "mvn",  "neg",  "negs", "orn",
    "orr",  "str",  "sub",  "subs", "tst"};

static const char *DATA_PROCESSING_INSTRUCTIONS[] = {
  "add", "adds", "and", "ands", "bic", "bics", "cmn",
  "cmp", "eon", "eor", "madd", "mneg", "mov", "movk", "movn", "movz", "msub", "mul",
  "mvn", "neg", "negs", "orn", "orr", "sub", "subs", "tst"
};

static const char *RN_RD_INSTRUCTIONS[] = {
  "and", "ands", "bic", "bics", "eon", "eor", "madd", "mneg", "mov", "msub",
  "mul", "mvn", "orn", "orr", "tst"
};

static const char *RN_RD_RM_INSTRUCTIONS[] = {"and",  "ands", "bic",  "bics",
                                              "eon",  "eor",  "madd", "mneg",
                                              "msub", "mul",  "orn",  "orr"};

static const char *OP1_IMMEDIATE[] = {
  "add", "adds", "cmn", "cmp", "movk", "movn", "movz", "neg", "negs", "sub", "subs"
};

static const char *CONDITION_BRANCHING_INSTRUCTIONS[] = {
  "b.al", "b.eq", "b.ge", "b.gt", "b.le", "b.lt", "b.ne"
};


// strcmp wrapper but with generic type for binary search
static int cmp_str(const void* a, const void* b) {
  const char *str1 = *(const char**) a;
  const char *str2 = *(const char**) b;
  return strcmp(str1, str2);
}

static bool is_rn_rd_instructions(char *command) {
  return (bsearch(&command, RN_RD_INSTRUCTIONS, 15, sizeof(*RN_RD_INSTRUCTIONS), cmp_str)) != NULL;
}

static bool is_rn_rd_rm_instructions(char *command) {
  return (bsearch(&command, RN_RD_RM_INSTRUCTIONS, 12,
                  sizeof(*RN_RD_RM_INSTRUCTIONS), cmp_str)) != NULL;
}

static bool is_op1_immediate_instruction(char *command) {
  return (bsearch(&command, OP1_IMMEDIATE, 11, sizeof(*OP1_IMMEDIATE), cmp_str)) != NULL;
}

bool is_bcond_instruction(char *command) {
  return (bsearch(&command, CONDITION_BRANCHING_INSTRUCTIONS, 7, sizeof(*CONDITION_BRANCHING_INSTRUCTIONS), &cmp_str )) != NULL;
}

static shift_t convert_string_to_shift_t(char *str) {
  if (strcmp(str, "lsr") == 0) return LSR;
  else if (strcmp(str, "lsl") == 0) return LSL;
  else if (strcmp(str, "asr") == 0) return ASR;
  else if (strcmp(str, "ror") == 0) return ROR;
  else {
    printf("Cannot convert string to valid shift type, line: 58\n");
    exit(1);}
}

static void add_optional_shift(parsed_line_t *parsed, tokenized_line_t tok, int num_concrete_ops) {
  if (tok.length - 1 == num_concrete_ops + 2) { // -1 for mnemonic, +2 for additional shift param
    parsed->instr.operand_count = num_concrete_ops + 2;
    parsed->instr.operands[num_concrete_ops] = (operand_t) {
      .type = OPERAND_SHIFT,
      .shift_type = convert_string_to_shift_t(tok.tokens[num_concrete_ops+1]),
    };
    parsed->instr.operands[num_concrete_ops+1] = (operand_t) {
      .type = OPERAND_IMMEDIATE,
      .immediate = strtol(tok.tokens[num_concrete_ops + 2] + 1, NULL, 0) 
    };
  } else {
    return;
  }
}

bool is_label(const char *str) {
  if (!(isalpha(str[0]) || str[0] == '_' || str[0] == '.')) {
    return false;
  }
  for (unsigned long i = 1; i < strlen(str); i++) {
    if (!(isalnum(str[i]) || str[i] == '$' || str[i] == '_' || str[i] == '.'  || str[i] == ':')) {
      return false;
    }
  }
  return true;
}

bool is_instruction(const char *str) {
  for (int i = 0; i < NR_INSTRUCTIONS; i++) {
    if (strcmp(ALL_INSTRUCTIONS[i], str ) == 0 ) {
      return true;
    }
  }
  return false;
}

// strtol(tok_line.tokens[1] + 1, NULL, 0),
static unsigned int get_reg_num(const char *regn) {
  if (strcmp(regn, "zr") == 0) {
    return 31;
  }
  return strtol(regn, NULL, 0);
}

static instruction_type_t get_instr_type(char *command) {
  if (strcmp(command, "ldr") == 0 || strcmp(command, "str") == 0) {
    return INSTR_LOAD_STORE;
  } else if (strcmp(command, "b") == 0 || is_bcond_instruction(command) || strcmp(command, "br") == 0) {
    return INSTR_BRANCH;
  } else if (bsearch(&command, DATA_PROCESSING_INSTRUCTIONS, 26, sizeof(*DATA_PROCESSING_INSTRUCTIONS), cmp_str)) {
    return INSTR_DATA_PROCESSING;
  } else {
    printf("Cannot parse line: 100\n");
    exit(1);
  }
}

static offset_type_t get_offset_type(tokenized_line_t tok) {
  if (tok.length == 3 && tok.tokens[2][0] != '[') {
    assert(strcmp(tok.tokens[0], "ldr") == 0);
    return LOAD_LITERAL;
  }

  char* arg1 = tok.tokens[2];
  char* arg2 = tok.tokens[3];

  if (arg1[strlen(arg1) - 1] == ']') {
    //MID: POST-INDEX OR UNSIGNED OFFSET
    if (tok.length == 4) {
      return POST_INDEX;
    } else {
      return UNSIGNED_OFFSET;  // without optional immediate 
    }
  } else if (arg2[strlen(arg2) - 1] == ']' || arg2[strlen(arg2) - 1] == '!')  {
    if (arg2[strlen(arg2) - 1] == '!') {
      return PRE_INDEX;
    } else if (is_immediate(arg2)) {
      return UNSIGNED_OFFSET;  // with optional immediate 
    } else {
      return REGISTER_OFFSET;
    }
  } else {
    printf("Cannot parse line: 129\n");
    exit(1); //ERROR
  }
}

static char* remove_closing_bracket(char* target) {
  //PRE: assume in: "[1234]"
  char *str = target + 1;
  char *closing_bracket = strchr(str, ']');
  if (closing_bracket) {
    *closing_bracket = '\0'; 
  }
  return str;
}

parsed_line_type_t get_line_type(char *command) {
  if (strcmp(command, ".int") == 0 ) { 
    return LINE_DIRECTIVE;
  } else if (is_instruction(command)) {
    return LINE_INSTRUCTION;
  } else if (is_label(command)) {
    return LINE_LABEL;
  }
  return SKIP;
}

// typedef struct {
//   char *curr_mnemonic; /* current mnemonic of the aliased instruction */
//   char *des_mnemonic;  /* desired mnemonic of the aliased instruction */
//   token_mnemonic_t
//       des_mnemonic_tok; /* desired mnemonic of the aliased instruction */
//   int rzr_idx; /* index of where to put the rzr operand (all of the alias
//                   instructions just take additional rzr operand) */
// } alias_item_t;

// #define ALIAS_INSTR_SIZE (sizeof(alias_conv_table) / sizeof(alias_item_t))

// static alias_item_t alias_conv_table[] = {
//     {"cmp", "subs", TOKEN_SUBS, 1},  // rzr → operand[0] → token[1]
//     {"cmn", "adds", TOKEN_ADDS, 1},  // rzr → operand[0] → token[1]
//     {"neg", "sub", TOKEN_SUB, 2},    // rzr → operand[1] → token[2]
//     {"negs", "subs", TOKEN_SUBS, 2}, // rzr → operand[1] → token[2]
//     {"tst", "ands", TOKEN_ANDS, 1},  // rzr → operand[0] → token[1]
//     {"mvn", "orn", TOKEN_ORN, 2},    // rzr → operand[1] → token[2]
//     {"mov", "orr", TOKEN_ORR, 2},    // rzr → operand[1] → token[2]
//     {"mul", "madd", TOKEN_MADD, 4},  // rzr → operand[3] → token[4]
//     {"mneg", "msub", TOKEN_MSUB, 4}, // rzr → operand[3] → token[4]
// };

// Tokenise by splitting given string into a its opcode + operand
static tokenized_line_t tokenize(char input[]) {
  char *delimiter = " ,\n";
  char *saveptr;
  int tok_counter = 0;
  
  char *tok = strtok_r(input, delimiter, &saveptr);  
  parsed_line_type_t line_type = (tok == NULL) ? SKIP : get_line_type(tok);;

  tokenized_line_t tokenized_line;
  tokenized_line.line_t = line_type;

  if (line_type == LINE_INSTRUCTION) {
    tokenized_line.mnemonic = get_mnemonic_token(tok);
  }

  while (tok != NULL && tok_counter < MAX_TOKENS_COUNT &&
         strcmp(tok, ";") != 0) {
    tokenized_line.tokens[tok_counter++] = tok;
    tok = strtok_r(NULL, delimiter, &saveptr);
  }
  tokenized_line.length = tok_counter;

  return tokenized_line;
}

// Main parse function into IR-format
parsed_line_t parse(char str_input[], u32 address, symbol_table_ptr_t table) {

  tokenized_line_t tok_line = tokenize(str_input);  //the line_type identifier, then the array of split strings
  //tok_line = {"ldr", "x3", "[x1", "#8]"}

  switch (tok_line.line_t) {
    case LINE_LABEL: {
      tok_line.tokens[0][strlen(tok_line.tokens[0]) - 1 ] = '\0';
      parsed_line_t parsed_label = {
        .type = LINE_LABEL, 
        .label.name = put_label(table, tok_line.tokens[0], address, true)
      };
      return parsed_label;
      break;
    } 

    case LINE_DIRECTIVE:  {
      parsed_line_t parsed_dir = {
        .type = LINE_DIRECTIVE, 
        .dir.value = strtol(tok_line.tokens[1], NULL, 0)
      };
      return parsed_dir;
      break;
    }

    case LINE_INSTRUCTION: {
      parsed_line_t parsed_instr;
      parsed_instr.type = LINE_INSTRUCTION;

      /* TODO: do we really have to malloc? */
      parsed_instr.instr.mnemonic = malloc(strlen(tok_line.tokens[0]) * sizeof(char));
      parsed_instr.instr.mnemonic_tok = tok_line.mnemonic;
      assert (parsed_instr.instr.mnemonic != NULL); //CHANGE THIS TO SOMETHING COOLER
      strcpy(parsed_instr.instr.mnemonic, tok_line.tokens[0]);
      parsed_instr.instr.instr_type = get_instr_type(tok_line.tokens[0]);
      //parsed_instr.instr.label_address = address;

      switch (parsed_instr.instr.instr_type) {
        case INSTR_BRANCH: {
          parsed_instr.instr.operand_count = 1;
          
          if (strcmp(tok_line.tokens[0], "b") == 0 || is_bcond_instruction(tok_line.tokens[0])) {
            //MID: tokens[1] is a literal
            if (is_label(tok_line.tokens[1])) {
              parsed_instr.instr.operands[0].type = OPERAND_LITERAL_LABEL;
              parsed_instr.instr.operands[0].literal_label = put_label(table, tok_line.tokens[1], UINT32_MAX, false);
            } else {
              parsed_instr.instr.operands[0].type = OPERAND_LITERAL_ADDRESS;
              parsed_instr.instr.operands[0].literal_address = strtol(tok_line.tokens[1], NULL, 0);
            }

          } else if (strcmp(tok_line.tokens[0], "br") == 0 ) {
            //MID: tokens[1] is xn register
            char *str = tok_line.tokens[1];
            u8 xn = strtol(str + 1, NULL, 0);
            parsed_instr.instr.operands[0] = (operand_t){
              .type = OPERAND_REGISTER, 
              .reg = {
                .is_64bit = true, 
                .reg_num = xn
              }
            };
          } else {
            printf("Invalid instruction: %s", tok_line.tokens[0]);
          }
          break;
        }
        
        case INSTR_LOAD_STORE: {
          parsed_instr.instr.operands[0] =
              (operand_t){.type = OPERAND_REGISTER,
                          .reg = {
                              .is_64bit = tok_line.tokens[1][0] == 'x',
                              .reg_num = get_reg_num(tok_line.tokens[1] + 1),
                          }};

          offset_type_t offset_t = get_offset_type(tok_line);

          switch (offset_t) {
            case (POST_INDEX): {
              parsed_instr.instr.operand_count = 3;
              parsed_instr.instr.operands[1] = (operand_t) {
                .type = OPERAND_MEMORY_POST_INDEX, 
                .reg = {
                  .is_64bit = true, 
                  .reg_num = strtol(remove_closing_bracket(tok_line.tokens[2] + 1), NULL, 0)
                }
              };

              parsed_instr.instr.operands[2] = (operand_t) {
                .type = OPERAND_SIGNED_IMMEDIATE, 
                .s_immediate = strtol(tok_line.tokens[3] + 1, NULL, 0) 
              };
              break;
            }
            
            case (PRE_INDEX): {
              parsed_instr.instr.operand_count = 3;
              parsed_instr.instr.operands[1] = (operand_t) {
                .type = OPERAND_MEMORY_PRE_INDEX, 
                .reg = {
                  .is_64bit = true, 
                  .reg_num = strtol(tok_line.tokens[2] + 2, NULL, 0)
                }
              };

              /* TODO: maybe add 1 to tokens[3] in remove_closing bracket? */
              parsed_instr.instr.operands[2] = (operand_t){
                  .type = OPERAND_SIGNED_IMMEDIATE,
                  .s_immediate = strtol(
                      remove_closing_bracket(tok_line.tokens[3]), NULL, 0)};
              break;
            }

            case (UNSIGNED_OFFSET): {
              parsed_instr.instr.operands[1] = (operand_t) {
                .type = OPERAND_MEMORY_UNSIGNED_OFFSET, 
                .reg = {
                  .is_64bit = true, 
                  .reg_num = strtol(remove_closing_bracket(tok_line.tokens[2] + 1), NULL, 0)
                }
              };

              if (tok_line.length == 3) {
                //MID: NO OPTIONAL IMM
                parsed_instr.instr.operand_count = 2;
              } else if (tok_line.length == 4){
                //MID: YES OPTIONAL IMM
                parsed_instr.instr.operand_count = 3;
                parsed_instr.instr.operands[2] = (operand_t){
                    .type = OPERAND_IMMEDIATE,
                    .immediate = strtol(
                        remove_closing_bracket(tok_line.tokens[3]), NULL, 0)};
              } else {
                printf("Cannot parse line: 313\n");
                exit(1); //ERROR
              }
              break;
            }
            case (REGISTER_OFFSET): {
              parsed_instr.instr.operand_count = 3;
              parsed_instr.instr.operands[1] = (operand_t) {
                .type = OPERAND_MEMORY_REGISTER_OFFSET, 
                .reg = {
                  .is_64bit = true, 
                  .reg_num = strtol(tok_line.tokens[2] + 2, NULL, 0)
                }
              };

              parsed_instr.instr.operands[2] = (operand_t){
                  .type = OPERAND_REGISTER,
                  .reg = {.is_64bit = true,
                          .reg_num = get_reg_num(
                              remove_closing_bracket(tok_line.tokens[3]))}};
              break;
            }
            case (LOAD_LITERAL): {
              parsed_instr.instr.operand_count = 2; 
              if (is_label(tok_line.tokens[2])) {
                parsed_instr.instr.operands[1].type = OPERAND_LITERAL_LABEL;
                parsed_instr.instr.operands[1].literal_label = put_label(table, tok_line.tokens[2], UINT32_MAX, false);
              } else {
                parsed_instr.instr.operands[1].type = OPERAND_LITERAL_ADDRESS;
                parsed_instr.instr.operands[1].literal_address = strtol(tok_line.tokens[2] + 1, NULL, 0);
              }
              break;
            }
            default: 
              printf("Cannot parse line: 350\n");
              exit(1);
              break;
          }
          break;
        }

        case INSTR_DATA_PROCESSING: {
          parsed_instr.instr.operands[0] = (operand_t){
              .type = OPERAND_REGISTER,
              .reg = {.is_64bit = tok_line.tokens[1][0] == 'x',
                      .reg_num = get_reg_num(tok_line.tokens[1] + 1)}};
          parsed_instr.instr.operand_count = 1;
          
          char *mnemonic = tok_line.tokens[0]; 
          //mul/mneg, madd/msub, mov, tst, 
          if (is_rn_rd_instructions(mnemonic)) {
            parsed_instr.instr.operands[1] = (operand_t){
                .type = OPERAND_REGISTER,
                .reg = {.is_64bit = tok_line.tokens[2][0] == 'x',
                        .reg_num = get_reg_num(tok_line.tokens[2] + 1)}};
            parsed_instr.instr.operand_count ++;

            if (is_rn_rd_rm_instructions(mnemonic)) {
              parsed_instr.instr.operands[2] = (operand_t){
                  .type = OPERAND_REGISTER,
                  .reg = {.is_64bit = tok_line.tokens[3][0] == 'x',
                          .reg_num = get_reg_num(tok_line.tokens[3] + 1)}};
              parsed_instr.instr.operand_count ++;

              if (strcmp("msub", mnemonic) == 0 || strcmp("madd", mnemonic) == 0 ) {
                parsed_instr.instr.operands[3] = (operand_t){
                    .type = OPERAND_REGISTER,
                    .reg = {.is_64bit = tok_line.tokens[4][0] == 'x',
                            .reg_num = get_reg_num(tok_line.tokens[4] + 1)}};
                parsed_instr.instr.operand_count ++;
              } else {
                add_optional_shift(&parsed_instr, tok_line, 3); // 3 concrete operands: 
              }
            } else {
              add_optional_shift(&parsed_instr, tok_line, 2); // 2 concrete operands
            }
          } else if (is_op1_immediate_instruction(mnemonic)) {
            if (strcmp(mnemonic, "movk") == 0 || strcmp(mnemonic, "movn") == 0 || strcmp(mnemonic, "movz") == 0) {
              parsed_instr.instr.operands[1] = (operand_t) {
                .type = OPERAND_IMMEDIATE,
                .immediate = strtol(tok_line.tokens[2] + 1, NULL, 0)
              };
              parsed_instr.instr.operand_count ++;
              add_optional_shift(&parsed_instr, tok_line, 2);

              //add ... cmp ...neg
            } else if (strcmp(mnemonic, "add") == 0 || strcmp(mnemonic, "adds") == 0 || strcmp(mnemonic, "sub") == 0 || strcmp(mnemonic, "subs") == 0
              || strcmp(mnemonic, "cmp") == 0 || strcmp(mnemonic, "cmn") == 0 || strcmp(mnemonic, "neg") == 0 || strcmp(mnemonic, "negs") == 0) {
              if(is_immediate(tok_line.tokens[2])) {
                parsed_instr.instr.operands[1] = (operand_t) {
                  .type = OPERAND_IMMEDIATE,
                  .immediate = strtol(tok_line.tokens[2] + 1, NULL, 0)
                };
                parsed_instr.instr.operand_count ++;
                add_optional_shift(&parsed_instr, tok_line, 2);

              } else {
                parsed_instr.instr.operands[1] = (operand_t){
                    .type = OPERAND_REGISTER,
                    .reg = {.is_64bit = tok_line.tokens[2][0] == 'x',
                            .reg_num = get_reg_num(tok_line.tokens[2] + 1)}};
                parsed_instr.instr.operand_count ++;

                if (strcmp(mnemonic, "add") == 0 || strcmp(mnemonic, "adds") == 0 || strcmp(mnemonic, "sub") == 0 || strcmp(mnemonic, "subs") == 0) {
                  if(is_immediate(tok_line.tokens[3])) {
                    parsed_instr.instr.operands[2] = (operand_t) {
                      .type = OPERAND_IMMEDIATE,
                      .immediate = strtol(tok_line.tokens[3] + 1, NULL, 0)
                    };
                    parsed_instr.instr.operand_count ++;
                  } else {
                    parsed_instr.instr.operands[2] = (operand_t){
                        .type = OPERAND_REGISTER,
                        .reg = {.is_64bit = tok_line.tokens[3][0] == 'x',
                                .reg_num =
                                    get_reg_num(tok_line.tokens[3] + 1)}};
                    parsed_instr.instr.operand_count ++;
                  }
                  add_optional_shift(&parsed_instr, tok_line, 3);
                } else {
                  add_optional_shift(&parsed_instr, tok_line, 2);
                }
              }
            } else {
              printf("Cannot parse line: 444\n");
              exit(1); //ERROR
            }
          } else {
            printf("Cannot parse line: 448\n");
            exit(1); //ERROR
          }
          break;
        }
        default:
        printf("Cannot parse line: 454\n");
          exit(1); //ERROR
      }
      return parsed_instr;
      break;
    }
    case SKIP: {
      parsed_line_t skip;
      skip.type = SKIP;
      return skip;
      break;
    }
    default: 
      printf("Cannot parse line: 466\n");
      exit(1);
  }

  parsed_line_t skip;
  skip.type = SKIP;
  return skip;
}
