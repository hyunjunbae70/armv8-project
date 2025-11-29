#include "assemble_load_store.h"
#include "assemble.h"
#include "../utils/bits_utils.h"
#include <assert.h>
#include <stdlib.h>

u32 assemble_load_store(instruction_IR_t *ps, u32 address) {
    /*
     * if literal for ldr:
     * check 1) address is 1MB within the address of instruction
     *       2) is 4 byte aligned
     */
    u32 instr = 0x0;
    operand_t rt = ps->operands[0];
    u8 sf = rt.reg.is_64bit;
    insert_bits_u32(&instr, 27, 28, 3); /* 0b11 */
    insert_bits_u32(&instr, 30, 30, sf);
    assert(rt.reg.reg_num <= MAX_REG_NUM);
    insert_bits_u32(&instr, 0, 4, rt.reg.reg_num);

    if (ps->mnemonic_tok == TOKEN_LDR) {
      operand_t addressing = ps->operands[1];
      switch (addressing.type) {
      case OPERAND_LITERAL_ADDRESS: {
        /* load literal instruction */
        u32 target_addr = addressing.literal_address;
        i32 offs_raw = target_addr - address;
        assert(abs(offs_raw) <=
               (1 << 20)); /* SPEC: must be within 1MB of the instruction */
        assert(offs_raw % 4 == 0); /* SPEC: must be 4-byte aligned */
        i32 offs = offs_raw >> 2;
        /* insert the simm19 offset to instruction 5-23 */
        insert_bits_u32(&instr, 5, 23, offs);
        return instr;
        break;
      }
      default:
        insert_bits_u32(&instr, 22, 22, 1);
        break; /* NOTE: the addressing will be handled after the if-else */
      }
    } else if (ps->mnemonic_tok == TOKEN_STR) {
      insert_bits_u32(&instr, 22, 22, 0);
    } else {
      fprintf(stderr, "Invalid load/store instruction mnemonic token: %d\n",
              ps->mnemonic_tok);
      exit(1);
    }

    insert_bits_u32(&instr, 31, 31, 1); /* for single data transfer bits */
    insert_bits_u32(&instr, 29, 29, 1); /* for single data transfer bits */

    /* unsigned immediate: set 24th bit to 1 if it is unsigned immedaite mode */
    operand_t xn = ps->operands[1];
    insert_bits_u32(&instr, 5, 9, xn.reg.reg_num);

    switch (xn.type) {
    case OPERAND_MEMORY_POST_INDEX:
    case OPERAND_MEMORY_PRE_INDEX: {
      /* set I (bit 11): 0 for post-index, 1 for pre-index */
      u32 i_bit = (xn.type == OPERAND_MEMORY_PRE_INDEX) ? 1 : 0;
      insert_bits_u32(&instr, 11, 11, i_bit);
      insert_bits_u32(&instr, 10, 10, 1);
      /* set bits 12-20 to simm9 */
      operand_t signedop = ps->operands[2];
      i32 simm9 = signedop.s_immediate;
      assert(simm9 > -256 && simm9 < 255);
      /* convert it to 9 bits while preserving its sign */
      insert_bits_u32(&instr, 12, 20, (u32)simm9);
      break;
    }
    case OPERAND_MEMORY_REGISTER_OFFSET: {
      insert_bits_u32(&instr, 21, 21, 1);
      insert_bits_u32(&instr, 13, 14, 3); /* 0b11 */
      insert_bits_u32(&instr, 11, 11, 1);
      operand_t xm = ps->operands[2];
      assert(xm.reg.reg_num <= MAX_REG_NUM);
      insert_bits_u32(&instr, 16, 20, xm.reg.reg_num);
      break;
    }
    case OPERAND_MEMORY_UNSIGNED_OFFSET: {
      /* bits 10-21 are imm 12 */
      u32 imm12_raw = 0;
      if (ps->operand_count == 3) {
        /* we have immediate as an offset */
        imm12_raw = ps->operands[2].immediate;
      }

      u32 imm12 = imm12_raw;
      if (sf) {
        /* Rt is 64-bit X-register */
        assert(imm12 % 8 == 0);
        imm12 >>= 3;
      } else {
        /* Rt is 32-bit W-register*/
        assert(imm12 % 4 == 0);
        imm12 >>= 2;
      }

      assert(imm12 <= 4095);
      insert_bits_u32(&instr, 10, 21, imm12);
      insert_bits_u32(&instr, 24, 24, 1);
      break;
    }
    default:
      fprintf(stderr, "Invalid addressing mode!\n");
      exit(1);
      break;
    }

    return instr;
}
