#include "assemble.h"
#include "../utils/bits_utils.h"
#include "../utils/hashmap.h"
#include "instruction_assembler.h"
#include "parser.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_INSTRUCTIONS_CAPACITY 64

static void free_instrs(parsed_line_t *parses, u32 size) {
  for (u32 i = 0; i < size; i++) {
    if (parses[i].type == LINE_INSTRUCTION) {
      free(parses[i].instr.mnemonic);
    }
  }
}

int label_conversion(symbol_table_ptr_t symbol_table, operand_t *operand) {
  if (operand->type == OPERAND_LITERAL_LABEL) {
    operand->literal_address =
        get_label_address(symbol_table, operand->literal_label);
    operand->type = OPERAND_LITERAL_ADDRESS;
    if (operand->literal_address == UINT32_MAX) {
      return EXIT_FAILURE;
    }
  } else if (operand->type != OPERAND_LITERAL_ADDRESS) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "[aj3124] Format: %s input.s output.bin\n", argv[0]);
    return EXIT_FAILURE;
  }

  FILE *in = fopen(argv[1], "r");
  FILE *out = fopen(argv[2], "wb");

  if (in == NULL || out == NULL) {
    fprintf(stderr, "[aj3124] Error while opening files.\n");
    return EXIT_FAILURE;
  }

  symbol_table_ptr_t symbol_table = create_table_ADT();

  if (symbol_table == NULL) {
    fprintf(stderr, "[aj3124] Error while creating symbol table.\n");
    fclose(in);
    fclose(out);
    return EXIT_FAILURE;
  }

  char *line = NULL;
  size_t len = 0;
  u32 address = 0;
  parsed_line_t *parses = malloc( sizeof(*parses) * INITIAL_INSTRUCTIONS_CAPACITY);
  unsigned int size = 0;
  unsigned int capacity = INITIAL_INSTRUCTIONS_CAPACITY;
  if (parses == NULL) {
    fprintf(stderr, "[avl24] Error while instruction array.\n");
    fclose(in);
    fclose(out);
    free_table(symbol_table);
    return EXIT_FAILURE;
  }

  while (getline(&line, &len, in) != -1) {
    if (size == capacity) {
      capacity <<= 1;
      parsed_line_t *tmp = realloc(parses, sizeof(*parses) * capacity);
      if (tmp == NULL) {
        fprintf(stderr, "[aj3124] Error while resizing instruction array.\n");
        fclose(in);
        fclose(out);
        free(line);
        free_table(symbol_table);
        free_instrs(parses, size);
        free(parses);
        return EXIT_FAILURE;
      }
      parses = tmp;
    }
    parses[size] = parse(line, address, symbol_table);
    if (parses[size].type != SKIP) {
      if (parses[size].type == LINE_INSTRUCTION ||
          parses[size].type == LINE_DIRECTIVE) {
        address += 4;
        if (address > MEMORY_SIZE) {
          fprintf(stderr, "WROTE TOO MUCH INTO MEMORY\n");
          fclose(in);
          fclose(out);
          free(line);
          free_table(symbol_table);
          free_instrs(parses, size+1); //MAYBE CHANGE +1
          free(parses);
          return EXIT_FAILURE;
        }
      } else if (parses[size].type != LINE_LABEL) {
        fprintf(stderr, "INCORRECT INSTRUCTION PARSE\n");
        fclose(in);
        fclose(out);
        free(line);
        free_table(symbol_table);
        free_instrs(parses, size+1); //MAYBE CHANGE +1
        free(parses);
        return EXIT_FAILURE;
      }
      size++;
    }
  }  
  free(line);
  // We can realloc the parses to fit its size exactly here
  address = 0;
  for (unsigned int i = 0; i < size; ++i) {
    switch (parses[i].type) {
      case LINE_DIRECTIVE:
        write_u32_le(out, assemble_instruction(&parses[i], address));
        address += 4;
        break;
      case LINE_INSTRUCTION: //TODO replace literal with value of label
        if (parses[i].instr.instr_type == INSTR_BRANCH && (!strcmp(parses[i].instr.mnemonic, "b") 
            || is_bcond_instruction(parses[i].instr.mnemonic))) {
          if (label_conversion(symbol_table, &parses[i].instr.operands[0])) {
            fprintf(stderr, "[aj3124] Error parsing `b` or `b.cond` wrong operand.\n");
            fclose(in);
            fclose(out);
            free(line);
            free_table(symbol_table);
            free_instrs(parses, size);
            free(parses);
            return EXIT_FAILURE;
          }
        } else if( parses[i].instr.instr_type == INSTR_LOAD_STORE && !strcmp(parses[i].instr.mnemonic, "ldr") 
            && (parses[i].instr.operands[1].type == OPERAND_LITERAL_LABEL || parses[i].instr.operands[1].type == OPERAND_LITERAL_ADDRESS)) {
          if (label_conversion(symbol_table, &parses[i].instr.operands[1])) {
            fprintf(stderr, "[aj3124] Error parsing `ldr` wrong operand.\n");
            fclose(in);
            fclose(out);
            free(line);
            free_table(symbol_table);
            free_instrs(parses, size);
            free(parses);
            return EXIT_FAILURE;
          }
        }
        u32 assembled_instr = assemble_instruction(&parses[i], address);
        write_u32_le(out, assembled_instr);
        address += 4;
        break;
      case SKIP: // SHOULD NEVER HAPPEN
        assert (false);
      case LINE_LABEL: //FALLTHROW:
      default:
        break;
    }
  }
  fclose(in);
  fclose(out);
  free_table(symbol_table);
  free_instrs(parses, size);
  free(parses);

  return EXIT_SUCCESS;
}
