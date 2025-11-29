#ifndef BRANCHES
#define BRANCHES

#include "../emulate.h"
/*
 * each instruction is 32-bit, little-endian format
 * op0 (bits 25-28) 101x is for branch
 * So, the branch instruction is:
 * instr[31-30] = xx
 * instr[29] = 0
 * instr[28-26] = 101
 * instr[25-0] = operand
 */

bool branch_instr(instruction instr);

#endif /* BRANCHES */
