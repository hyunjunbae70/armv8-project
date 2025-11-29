#include "machine.h"
#include "../utils/bits_utils.h"
#include "emulate.h"
#include "execute/branches.h"
#include "execute/halt.h"
#include "execute/immediate_instructions.h"
#include "execute/load_store.h"
#include "execute/register_instruction.h"
#include <inttypes.h>
#include <stdio.h>

#define DP_IMM_BIT_PATTERN_1 0x8
#define DP_IMM_BIT_PATTERN_2 0x9
#define DP_REG_BIT_PATTERN_1 0x5
#define DP_REG_BIT_PATTERN_2 0xd
#define LS_BIT_PATTERN_1 0x4
#define LS_BIT_PATTERN_2 0xc
#define LS_BIT_PATTERN_3 0x6
#define LS_BIT_PATTERN_4 0xe
#define B_BIT_PATTERN_1 0xa
#define B_BIT_PATTERN_2 0xb


extern machine_t machine;

static instruction fetch(machine_t *m) {
  /* instructions are stored in little-endian format in the memory */
  /* so, reverse the byte order while fetching the instruction */
  u8 instr1 = m->memory[m->PC];     /* least significant byte */
  u8 instr2 = m->memory[m->PC + 1]; /* mid bytes */
  u8 instr3 = m->memory[m->PC + 2]; /* mid bytes */
  u8 instr4 = m->memory[m->PC + 3]; /* most significant byte*/

  instruction instr = 0x0;
  insert_bits_u32(&instr, 0, 7, instr1);
  insert_bits_u32(&instr, 8, 15, instr2);
  insert_bits_u32(&instr, 16, 23, instr3);
  insert_bits_u32(&instr, 24, 31, instr4);

  return instr;
}

static bool decode_and_execute(instruction instr) {
  if (halt_instr(instr))
    return FALSE;

  u8 op0 = (u8)extract_bits_u32(instr, 25, 28);
  switch (op0) {
  case DP_IMM_BIT_PATTERN_1:
  case DP_IMM_BIT_PATTERN_2:
    /* data processing (immediate) */
    immediate_exectution(instr);
    break;
  case DP_REG_BIT_PATTERN_1:
  case DP_REG_BIT_PATTERN_2:
    /* data processing (register) */
    register_execute(instr);
    break;
  case LS_BIT_PATTERN_1:
  case LS_BIT_PATTERN_2:
  case LS_BIT_PATTERN_3:
  case LS_BIT_PATTERN_4:
    /* loads and stores */
    execute_load_store(instr);
    break;
  case B_BIT_PATTERN_1:
  case B_BIT_PATTERN_2:
    /* branches */
    if (!branch_instr(instr))
      return false;
    break;
  default:
    fprintf(stderr, "Invalid instruction op0\n");
    return FALSE;
  }
  return TRUE;
}

static void init_machine(machine_t *machine) {
  machine->PC = START_INSTR_ADDR;
  machine->pstate.Z = TRUE;
  machine->pstate.C = FALSE;
  machine->pstate.N = FALSE;
  machine->pstate.V = FALSE;
}

void run_machine(machine_t *machine) {

  while (TRUE) {
    /* fetch instruction */
    reg old_pc = machine->PC;
    instruction instr = fetch(machine);
    /* decode and execute instruction */
    if (!decode_and_execute(instr)) /* if the instruction couldn't be parsed (or
    it is the halt instruction) break */
      break;

    /*
     * increment PC by instruction byte size after so that PC is pointing
     *  to the instruction currently being executed (the same instruction that
     * has been) fetched and decoded.
     */
    if (machine->PC == old_pc)
      machine->PC += sizeof(instruction);
  }
}

void shutdown_machine(machine_t *machine, FILE *out_stream) {
  fprintf(out_stream, "Registers:\n");
  for (int i = 0; i < REG_COUNT; i++) {
    fprintf(out_stream, "X%02d    = %016" PRIx64 "\n", i, machine->regs[i]);
  }

  fprintf(out_stream, "PC     = %016" PRIx64 "\n", machine->PC);
  fprintf(out_stream, "PSTATE : %c%c%c%c\n", machine->pstate.N ? 'N' : '-',
          machine->pstate.Z ? 'Z' : '-', machine->pstate.C ? 'C' : '-',
          machine->pstate.V ? 'V' : '-');
  fprintf(out_stream, "Non-zero memory:\n");
  for (int addr = 0; addr < MEMORY_SIZE; addr += 4) {
    /* TODO: move this to bits_utils? */
    u32 data = (u32)(machine->memory[addr]) |
               (u32)(machine->memory[addr + 1] << 8) |
               (u32)(machine->memory[addr + 2] << 16) |
               (u32)(machine->memory[addr + 3] << 24);
    if (data != 0) {
      fprintf(out_stream, "0x%" PRIx32 ": 0x%" PRIx32 "\n", addr, data);
    }
  }
}

static bool machine_load_program_file(machine_t *machine, FILE *file) {
  u8 *mem = machine->memory + machine->PC;
  /* load memory with bytes fetched from the image file */
  const size_t ret_code = fread(mem, 1, MEMORY_SIZE, file);
  if (ret_code == 0 && ferror(file)) {
    return FALSE;
  }

  return TRUE;
}

void machine_load_program(machine_t *machine, const char *prg) {
  init_machine(machine);
  FILE *infile = fopen(prg, "rb");
  if (!infile) {
    fprintf(stderr, "An error occured while trying to read the program file.");
    return;
  }

  bool read = machine_load_program_file(machine, infile);
  if (!read) {
    fprintf(stderr, "Error reading %s\n", prg);
  }
  fclose(infile);
}
