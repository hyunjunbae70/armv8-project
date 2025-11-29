#include "assemble_branch.h"
#include "../utils/bits_utils.h"
#include "assemble.h"
#include <assert.h>
#include <stdlib.h>

#define BRANCH_OPC 0x14000000
#define BRANCH_SIMM_26_BITMASK 0x03ffffff
#define BRANCH_REG_MISC_BITS 0x21f

#define BRANCH_AL_ENCODING 0xe
#define BRANCH_EQ_ENCODING 0x0
#define BRANCH_GQ_ENCODING 0xa
#define BRANCH_GT_ENCODING 0xc
#define BRANCH_LE_ENCODING 0xd
#define BRANCH_LT_ENCODING 0xb
#define BRANCH_NE_ENCODING 0x1

static inline bool is_conditional_branch_token(token_mnemonic_t token) {
  return (token == TOKEN_B_AL || token == TOKEN_B_EQ || token == TOKEN_B_GQ ||
          token == TOKEN_B_GT || token == TOKEN_B_LE || token == TOKEN_B_LT ||
          token == TOKEN_B_NE);
}

u32 assemble_branch(instruction_IR_t *ps, u32 address) {
  u32 instr = BRANCH_OPC;

  if (ps->mnemonic_tok == TOKEN_B) {
    i32 raw_offset = ps->operands[0].literal_address - address;
    i32 offset = raw_offset >> 2;

    assert(offset > -(1 << 25) && offset <= (1 << 25));
    /* set the least significant 26-bits of the instruction to the offset */
    i32 simm26 = (i32)(offset & BRANCH_SIMM_26_BITMASK);
    instr |= simm26;
  } else if (ps->mnemonic_tok == TOKEN_BR) {
    /* unconditional branch with address at register */
    insert_bits_u32(&instr, 30, 31, 3);
    insert_bits_u32(&instr, 16, 25, BRANCH_REG_MISC_BITS);
    u32 addr_reg = ps->operands[0].reg.reg_num;
    assert(addr_reg <= MAX_REG_NUM);
    insert_bits_u32(&instr, 5, 9, addr_reg);
  } else if (is_conditional_branch_token(ps->mnemonic_tok)) {
    /* conditional branch with literal offset and a condition encoding */
    insert_bits_u32(&instr, 30, 31, 1);

    i32 raw_offset = ps->operands[0].literal_address - address;
    i32 offset = raw_offset >> 2;

    assert(offset > -(1 << 18) && offset <= (1 << 18));
    insert_bits_u32(&instr, 5, 23, offset);

    /* cond is bits 0-3 */
    u8 encoding;
    u8 branch_encodings[] = {BRANCH_AL_ENCODING, BRANCH_EQ_ENCODING,
                             BRANCH_GQ_ENCODING, BRANCH_GT_ENCODING,
                             BRANCH_LE_ENCODING, BRANCH_LT_ENCODING,
                             BRANCH_NE_ENCODING};

    /* utilize representing enum as an int */
    /* PRE: The branch token encodings are listed IN ORDER as they appear in the
     * branch_encodings array */
    unsigned int encoding_idx = (unsigned int)ps->mnemonic_tok;
    assert(encoding_idx >= TOKEN_B_AL && encoding_idx <= TOKEN_B_NE);
    encoding = branch_encodings[encoding_idx - TOKEN_B_AL];
    insert_bits_u32(&instr, 0, 3, encoding);
  } else {
    fprintf(stderr, "Invalid branch instruction mnemonic token: %d\n",
            ps->mnemonic_tok);
    exit(1);
  }
  return instr;
}
