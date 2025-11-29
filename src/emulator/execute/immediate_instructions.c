#include "immediate_instructions.h"
#include "../../defs.h"
#include "../../utils/bits_utils.h"
#include "../emulate.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define OPI_ARITH 0x2
#define OPI_WIDE_MOVE 0x5

#define WIDE_MOV_N_OPC 0x0
#define WIDE_MOV_Z_OPC 0x2
#define WIDE_MOV_K_OPC 0x3

void immediate_exectution(instruction instr) {
  // fprintf(stderr, "%u we are immediate\n", instr);

  bool sf = check_bit_u32(instr, SF_BIT_IMM);
  u32 opc = extract_bits_u32(instr, OPC_START_IMM, OPC_END_IMM);
  u32 opi = extract_bits_u32(instr, OPI_START_IMM, OPI_END_IMM);

  u32 rd_index = extract_bits_u32(instr, RD_START_IMM, RD_END_IMM);
  // reg *rd = &machine.regs[rd_index];

  if (opi == OPI_ARITH) {
    bool sh = check_bit_u32(instr, SH_BIT_IMM);
    u32 imm12 = extract_bits_u32(instr, IMM12_START_IMM, IMM12_END_IMM);
    u32 rn_index = extract_bits_u32(instr, RN_START_IMM, RN_END_IMM);
    // reg *rn = &machine.regs[rn_index];

    u64 operand = sh ? logical_shift_left((u64)imm12, 12) : (u64)imm12;
    u64 lhs = is_ZR(rn_index) ? 0 : machine.regs[rn_index];

    // If SF is 0 then bit-width for all registers is 32 bits.
    if (!sf) {
      lhs = zero_upper_32(lhs);
      operand = zero_upper_32(operand);
    }

    bool update_flags = (opc & 1);
    bool is_sub = (opc >> 1);
    u64 result = is_sub ? (lhs - operand) : (lhs + operand);

    if (update_flags) {
      machine.pstate.N = check_bit_u64(result, sf ? 63 : 31);
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

    // Writing the result to the destination register
    if (rd_index != REG_COUNT) {
      machine.regs[rd_index] = sf ? result : zero_upper_32(result);
    }
  } else if (opi == OPI_WIDE_MOVE) {
    // Wide move immediate instruction
    u32 hw = extract_bits_u32(instr, HW_START_IMM, HW_END_IMM);
    u32 imm16 = extract_bits_u32(instr, IMM16_START_IMM, IMM16_END_IMM);

    u32 shift = hw * 16;
    u64 operand = logical_shift_left(imm16, shift);

    if (!sf && hw > 1) {
      fprintf(stderr, "[aj3124] Invalid instruction format: Wide move "
                      "immediate with 32-bit operand.\n");
      exit(1);
    }

    u64 result = 0;

    switch (opc) {
    case WIDE_MOV_N_OPC: // MOVN
      result = ~operand;
      break;
    case WIDE_MOV_Z_OPC: // MOVZ
      result = operand;
      break;
    case WIDE_MOV_K_OPC: // MOVK
      result = (rd_index == REG_COUNT) ? 0 : machine.regs[rd_index];
      insert_bits_u64(&result, shift, shift + 15, imm16);
      break;
    default:
      fprintf(stderr, "[aj3124] Invalid instruction format: Unsupported opcode "
                      "for wide move immediate.\n");
      exit(1);
    }

    // Writing the result to the destination register
    if (rd_index != REG_COUNT) {
      machine.regs[rd_index] = sf ? result : zero_upper_32(result);
    }
  }
}
