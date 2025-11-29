#include "instruction_assembler.h"
#include "assemble.h"
#include "assemble_branch.h"
#include "assemble_dp.h"
#include "assemble_load_store.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static instruction_assembler_fun assembler_functions[3] = {
    assemble_data_processing,
    assemble_load_store,
    assemble_branch,
};

static u32 assemble_directive(directive_IR_t *ps) { return (u32)(ps->value); }

u32 assemble_instruction(parsed_line_t *ps, u32 address) {
  switch (ps->type) {
  case LINE_INSTRUCTION: {
    instruction_IR_t instr = ps->instr;
    assert(instr.instr_type >= 0 &&
           instr.instr_type <= (sizeof(assembler_functions) /
                                sizeof(instruction_assembler_fun)));
    return assembler_functions[instr.instr_type](&instr, address);
  }
  case LINE_DIRECTIVE: {
    directive_IR_t dir = ps->dir;
    return assemble_directive(&dir);
  }
  case LINE_LABEL:
    fprintf(stderr, "Invalid parsed line type: Labels should not be in parsed "
                    "lines, try resolving labels first!\n");
    exit(1);
    break;
  default:
    fprintf(stderr, "Invalid parsed line type!\n");
    exit(1);
    break;
  }
}
