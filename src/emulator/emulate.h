#ifndef EMULATE_H
#define EMULATE_H

#include "../defs.h"
#include <stdbool.h>
#include <stdio.h>

#define MEMORY_SIZE (1 << 21)
#define ZR 0
#define TERMINATE_OP 0x8a000000

#define START_INSTR_ADDR 0x0

#define REG_COUNT 31

typedef u64 reg;                 /* represents a 64-bit register */
typedef reg reg_file[REG_COUNT]; /* 31 general purpose registers */
typedef u32 instruction;         /* 32 bit instruction */
typedef struct {
  bool N, Z, C, V;
} PSTATE;

typedef struct {
  u8 memory[MEMORY_SIZE]; /* emulator memory (2^21 bytes) */
  reg PC;                 /* Program counter register */
  reg SP;                 /* Stack pointer register */
  PSTATE pstate; /* Program state register (contains condition flags) */
  reg_file regs; /* Register file - 31 general purpose registers */
} machine_t;

/* the main ARMv8 state */
/* machine should be defined in emulate.c */
extern machine_t machine;

#endif
