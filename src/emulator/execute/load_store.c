#include "load_store.h"
#include "../../defs.h"
#include "../../utils/bits_utils.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

static void check_memory_in_bounds(u64 address, int num_bytes) {
  if (address + num_bytes > MEMORY_SIZE) {
    printf("Memory access out of bounds at %" PRIx64 "\n", address);
    exit(1);
  }
}

static void store(u64 target_address, u32 target_register, int num_bytes) {
  reg source_reg_bits = machine.regs[target_register];
  check_memory_in_bounds(target_address, num_bytes);
  for (int i = 0; i < num_bytes; i++) {
    machine.memory[target_address + i] =
        (u8)extract_bits_u64(source_reg_bits, 8 * i, 8 * (i + 1) - 1);
  }
}

static u64 calculate_offset(instruction instr, u64 target, bool in_64) {
  u32 xn = extract_bits_u32(instr, XN_START, XN_END); // base address reg
  u64 base_addr = machine.regs[xn];

  if (check_bit_u32(instr, UNSIGNED_OFFSET_BIT)) {
    // MID: Unsigned Offset mode
    u64 uoffset;
    u32 imm12;
    imm12 = extract_bits_u32(instr, OFFSET_START, OFFSET_END);

    uoffset = (u64)imm12 * (in_64 ? 8 : 4);
    target = base_addr + uoffset;

  } else if (check_bit_u32(instr, REGISTER_OFFSET_BIT)) {
    // MID: Register offset mode
    reg xm = extract_bits_u32(instr, XM_START, XM_END); // offset address reg
    u64 offset_addr = machine.regs[xm];
    target = base_addr + offset_addr;

  } else if (check_bit_u32(instr, I_BIT)) {
    // MID: Pre-Indexed mode
    u32 simm9 = extract_bits_u32(instr, SIMM9_START, SIMM9_END);
    target = base_addr + sign_extend(simm9, 9);
    machine.regs[xn] = target;

  } else {
    // MID: Post-Indexed mode
    u32 simm9 = extract_bits_u32(instr, SIMM9_START, SIMM9_END);
    target = base_addr;
    machine.regs[xn] = base_addr + sign_extend(simm9, 9);
  };

  return target;
}

void execute_load_store(instruction instr) {
  // PRE: instr(OP0) = x1x0
  u32 op0 = extract_bits_u32(instr, OP0_START, OP0_END);
  assert(op0 == OP0a || op0 == OP0b || op0 == OP0c || op0 == OP0d);

  bool sf = check_bit_u32(instr, SF_BIT);             // sf = 1: 64bit
  u32 rt = extract_bits_u32(instr, RT_START, RT_END); // target register
  u64 target_addr = 0;                                // Transfer address
  bool load_literal = 0;                              // Load literal flag

  // Calculate offset depending on instruction
  if (check_bit_u32(instr, SINGLE_TRANSFER_BIT)) {
    target_addr = calculate_offset(instr, target_addr, sf);
  } else {
    // MID: Load Literal, set load_literal flag to true
    load_literal = true;
    u32 simm19 = extract_bits_u32(instr, SIMM19_START, SIMM19_END);
    target_addr = machine.PC + sign_extend(simm19, 19) * 4;
  }

  // Store and Load is split up into two branches for both 32 and 64 bit
  if (sf) {
    // MID: Rt is 64-bit
    if (check_bit_u32(instr, OPERATION_BIT) || load_literal) {
      // MID: load operation
      u64 value = 0;
      check_memory_in_bounds(target_addr, 8);
      for (int i = 0; i < 8; i++) {
        value |= ((u64)machine.memory[target_addr + i]) << (8 * i);
      }
      machine.regs[rt] = value;
    } else {
      store(target_addr, rt, 8);
    }

  } else {
    // MID: Rt is 32-bit
    if (check_bit_u32(instr, OPERATION_BIT) || load_literal) {
      // MID: load operation
      u32 value = 0;
      check_memory_in_bounds(target_addr, 4);
      for (int i = 0; i < 4; i++) {
        value |= ((u32)machine.memory[target_addr + i]) << (8 * i);
      }
      machine.regs[rt] = (reg)value;
    } else {
      store(target_addr, rt, 4);
    }
  }
}
