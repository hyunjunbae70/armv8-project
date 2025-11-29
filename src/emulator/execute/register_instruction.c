#include "register_instruction.h"
#include "../../defs.h"
#include "../../utils/bits_utils.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void shift(u64 *b, u32 shift, u32 operand, bool is_arith, bool sf) {
  switch (shift) {
  case OPP_LSL: // shift left
    *b = (*b) << operand;
    if (!sf)
      *b = zero_upper_32(*b);
    break;
  case OPP_LSR:           // shift right logic
    *b = (*b) >> operand; // & ((1ULL << (BITS_WIDTH(sf) - operand)) - 1);
    break;
  case OPP_ASR: { // shift right arithmetic
    u64 to_insert = (*b) >> operand;
    if (!sf) {
      if ((i32)(*b) < 0) {
        *b = UINT32_MAX;
        insert_bits_u64(b, 0, MSB_32 - operand, to_insert);
        // *b = zero_upper_32(*b);
      } else {
        *b = (*b) >> operand;
      }
    } else {
      if ((i64)(*b) < 0) {
        *b = UINT64_MAX;
        insert_bits_u64(b, 0, MSB_64 - operand, to_insert);
      } else {
        *b = (*b) >> operand;
      }
    }
    break;
  }
  case OPP_ROR:     // rotate right
    if (is_arith) { // should be only for logic
      fprintf(stderr, "no shift 11 for arithmetic (no rotate right)  \n");
      exit(1);
    } else {
      *b = (*b >> operand) | (*b << (BITS_WIDTH(sf) - operand));
    }
    break;
  default:
    assert(false);
  }
}

static u64 arithmetic(u64 lhs, u64 operand, u32 opc, bool sf) {
  bool update_flags = (opc & 1);
  bool is_sub = (opc >> 1);
  u64 result = is_sub ? (lhs - operand) : (lhs + operand);

  if (update_flags) {
    machine.pstate.N = check_bit_u64(result, sf ? MSB_64 : MSB_32);
    machine.pstate.Z = (sf ? result == 0 : zero_upper_32(result) == 0);

    if (!is_sub) {
      machine.pstate.C =
          (lhs > (sf ? UINT64_MAX - operand : UINT32_MAX - operand));
      machine.pstate.V =
          (sf ? ((i64)lhs > 0 && (i64)operand > 0 && (i64)result < 0) ||
                    ((i64)lhs < 0 && (i64)operand < 0 && (i64)result > 0)
              : ((i32)lhs > 0 && (i32)operand > 0 && (i32)result < 0) ||
                    ((i32)lhs < 0 && (i32)operand < 0 && (i32)result > 0));
    } else {
      machine.pstate.C = (lhs >= operand);
      machine.pstate.V =
          (sf ? ((i64)lhs > 0 && (i64)operand < 0 && (i64)result < 0) ||
                    ((i64)lhs < 0 && (i64)operand > 0 && (i64)result > 0)
              : ((i32)lhs > 0 && (i32)operand < 0 && (i32)result < 0) ||
                    ((i32)lhs < 0 && (i32)operand > 0 && (i32)result > 0));
    }
  }
  return result;
}

static u64 logic(u64 a, u64 b, u32 opc, bool sf) {
  switch (opc) {
  case OPP_AND: // and
    return (a & b);
  case OPP_OR: // or
    return (a | b);
  case OPP_XOR: // xor
    return (a ^ b);
  case OPP_AND_FLAGS: { // and with condition flags
    u64 result = a & b;

    machine.pstate.N = check_bit_u64(result, sf ? MSB_64 : MSB_32);
    machine.pstate.Z = (sf ? result == 0 : zero_upper_32(result) == 0);
    machine.pstate.C = 0;
    machine.pstate.V = 0;

    return result;
  }
  default:
    assert(false);
  }
}

u64 read_reg_from_instr(u32 instr, int start, int end) {
  int index = extract_bits_u32(instr, start, end);
  if (index < 31) {
    return machine.regs[index];
  } else {
    return ZR;
  }
}

// PRE: Is a register instruction; op0 = x101
void register_execute(instruction instr) {
  assert(extract_bits_u32(instr, OP0_START_REG, OP0_END_REG) == OP1_REGISTER ||
         extract_bits_u32(instr, OP0_START_REG, OP0_END_REG) == OP2_REGISTER);
  // REPLACE ABOVE WITH OP1 when defined

  bool sf = check_bit_u32(instr, SF_BIT_REG);
  u32 opc = extract_bits_u32(instr, OPC_START_REG, OPC_END_REG);
  bool is_arith = check_bit_u32(instr, OPR_FIRST_BIT_REG);
  u32 shift_op = extract_bits_u32(instr, SHIFT_START_REG, SHIFT_END_REG);
  bool negate = check_bit_u32(instr, NEGATE_BIT_REG);

  reg a = read_reg_from_instr(instr, RN_START_REG, RN_END_REG);
  reg b = read_reg_from_instr(instr, RM_START_REG, RM_END_REG);

  u32 res_index = extract_bits_u32(instr, RD_START_REG, RD_END_REG);
  u64 result = 0;

  if (!sf) {
    a = zero_upper_32(a);
    b = zero_upper_32(b);
  }

  if (check_bit_u32(instr, M_BIT)) { // multiply
    if (extract_bits_u32(instr, OP_M_START, OP_M_END) != OP_M_REGISTER) {
      fprintf(stderr, "wrong opcode for multiply \n");
      exit(1);
    }
    reg c = read_reg_from_instr(instr, RA_START_REG, RA_END_REG);
    reg aMb = a * b;
    if (!sf) {
      c = zero_upper_32(c);
      aMb = zero_upper_32(aMb);
    }
    // 2 * X, because x=1 -> 10 (sub); x=0 -> 00 (add)
    result = arithmetic(c, aMb, extract_bits_u32(instr, X_BIT, X_BIT) << 1, sf);
  } else { // arithmetic or logic
    u32 operand = extract_bits_u32(instr, OPERAND_START_REG, OPERAND_END_REG);
    assert(operand < BITS_WIDTH(sf));
    // perform shift:
    if (operand)
      shift(&b, shift_op, operand, is_arith, sf);

    if (is_arith) { // arithmetic
      if (negate) { // CHECK: negate should be 0
        fprintf(stderr, "arith shouldn't be negate - \n");
        exit(1);
      }
      result = arithmetic(a, b, opc, sf);

    } else { // logic
      if (negate) {
        b = ~b;
        if (!sf) {
          b = zero_upper_32(b);
        }
      }
      result = logic(a, b, opc, sf);
    }
  }
  if (res_index != REG_COUNT) {
    machine.regs[res_index] = sf ? result : zero_upper_32(result);
  }
}
