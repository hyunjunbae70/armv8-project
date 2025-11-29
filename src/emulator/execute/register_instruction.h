#ifndef REGISTER_INSTRUCION
#define REGISTER_INSTRUCION

#include "../emulate.h"
#include <stdio.h>

#define MSB_64 63
#define MSB_32 31

#define OP1_REGISTER 0x5
#define OP2_REGISTER 0xd
#define OP0_END_REG 28
#define OP0_START_REG 25

#define SF_BIT_REG 31
#define OPC_END_REG 30
#define OPC_START_REG 29
#define M_BIT 28
#define OPR_FIRST_BIT_REG 24
#define SHIFT_END_REG 23
#define SHIFT_START_REG 22
#define NEGATE_BIT_REG 21
#define RM_END_REG 20
#define RM_START_REG 16
#define RN_END_REG 9
#define RN_START_REG 5
#define RD_END_REG 4
#define RD_START_REG 0

// for multiply only:
#define X_BIT 15
#define RA_END_REG 14
#define RA_START_REG 10

#define OP_M_REGISTER 0xd8
#define OP_M_END 30
#define OP_M_START 21

// for arithmetic and logic only:
#define OPERAND_END_REG 15
#define OPERAND_START_REG 10

#define BITS_WIDTH(X) ((X) ? 64 : 32)

typedef enum { OPP_LSL, OPP_LSR, OPP_ASR, OPP_ROR } ShiftOp;

typedef enum { OPP_ADD, OPP_ADD_FLAGS, OPP_SUB, OPP_SUB_FLAGS } ArithOp;

typedef enum { OPP_AND, OPP_OR, OPP_XOR, OPP_AND_FLAGS } LogicOp;

/**
 * Executes a register instruction.
 *
 * @param instr PRE: Is a register instruction; op0 = x101
 */
void register_execute(instruction instr);

#endif
