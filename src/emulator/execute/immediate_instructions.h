#ifndef IMMEDIATE_INSTRUCTIONS
#define IMMEDIATE_INSTRUCTIONS

#include "../emulate.h"
#include <stdio.h>

// Bit indices for immediate instructions

#define SF_BIT_IMM 31

#define OPC_START_IMM 29
#define OPC_END_IMM 30

#define OPI_START_IMM 23
#define OPI_END_IMM 25

// For arithmetic immediate instructions
#define SH_BIT_IMM 22

#define IMM12_START_IMM 10
#define IMM12_END_IMM 21

#define RN_START_IMM 5
#define RN_END_IMM 9

#define RD_START_IMM 0
#define RD_END_IMM 4

// For wide move immediate instructions
#define HW_START_IMM 21
#define HW_END_IMM 22

#define IMM16_START_IMM 5
#define IMM16_END_IMM 20

extern void immediate_exectution(instruction instr);

#endif
