#include "assemble_dp.h"
#include "../utils/bits_utils.h"
#include "assemble.h"
#include <assert.h>
#include <stdlib.h>

/* Arithmetic instructions: opc|100|010 */
#define ADD_IMM_OPCODE 0x22
#define ADDS_IMM_OPCODE 0x62
#define SUB_IMM_OPCODE 0xA2
#define SUBS_IMM_OPCODE 0xE2

/* Wide Move instructions: opc|100|101 */
#define MOVN_OPCODE 0x25
#define MOVZ_OPCODE 0xA5
#define MOVK_OPCODE 0xE5

/* Arithmetic instructions: opc|0|101|1xx0 (opr: 1|shift|0) - shift is default
 * set to 00 */
#define ADD_REG_OPCODE 0x58
#define ADDS_REG_OPCODE 0x158
#define SUB_REG_OPCODE 0x258
#define SUBS_REG_OPCODE 0x358

/* Logical instructions: opc|0|101|0xxx (opr: 0|shift|N) - shift is default
 * set to 00 */
#define AND_OPCODE 0x50
#define BIC_OPCODE 0x51
#define ORR_OPCODE 0x150
#define ORN_OPCODE 0x151
#define EOR_OPCODE 0x250
#define EON_OPCODE 0x251
#define ANDS_OPCODE 0x350
#define BICS_OPCODE 0x351

/* Multiply instructions: opc|1|101|1000 */
#define MADD_OPCODE 0xd8
#define MSUB_OPCODE 0xd8

static inline bool is_dp_arith_token(token_mnemonic_t token) {
  return (token == TOKEN_ADD || token == TOKEN_ADDS || token == TOKEN_SUB ||
          token == TOKEN_SUBS);
}

static inline bool is_dp_logical_token(token_mnemonic_t token) {
  return (token == TOKEN_AND || token == TOKEN_ANDS || token == TOKEN_BIC ||
          token == TOKEN_BICS || token == TOKEN_EOR || token == TOKEN_ORR ||
          token == TOKEN_EON || token == TOKEN_ORN);
}

static inline bool is_dp_mult_token(token_mnemonic_t token) {
  return (token == TOKEN_MADD || token == TOKEN_MSUB);
}

static inline bool is_dp_mov_token(token_mnemonic_t token) {
  return (token == TOKEN_MOVK || token == TOKEN_MOVN || token == TOKEN_MOVZ);
}

typedef struct {
  token_mnemonic_t mnemonic; /* instruction mnemonic */
  u32 opcode;                /* necessary opcode bits for the instruction */
  bool reg;                  /* whether a reg instruction or imm */
} dp_instr_info_t;

static dp_instr_info_t dp_instrs[] = {
    /* ======== IMMEDIATE INSTRUCTIONS ======== */

    /* Arithmetic instructions */
    /* Has syntax: opc <Rd>, <Rn>, #<imm>{, lsl #(0|12)} */
    /* opcode: opc|100|010 */
    {TOKEN_ADD, ADD_IMM_OPCODE << 23, false},
    {TOKEN_ADDS, ADDS_IMM_OPCODE << 23, false},
    {TOKEN_SUB, SUB_IMM_OPCODE << 23, false},
    {TOKEN_SUBS, SUBS_IMM_OPCODE << 23, false},

    /* Wide Move instructions */
    /* Has syntax: opc <Rd>, #<imm>{, lsl #<imm>} */
    /* opcode: opc|100|101 */
    {TOKEN_MOVN, MOVN_OPCODE << 23, false},
    {TOKEN_MOVZ, MOVZ_OPCODE << 23, false},
    {TOKEN_MOVK, MOVK_OPCODE << 23, false},

    /* ======== REGISTER INSTRUCTIONS ======== */

    /* NOTE: sets shift value to 00 by default (for arithmetic and logical)
     * Therefore, if there is a shift argument the shift bit fields will be set
     * accordingly
     */
    /* Arithmetic instructions*/
    /* Has syntax: opc <Rd>, <Rn>, <Rm>{, <shift> #<imm>} */
    /* opcode: opc|0|101|1xx0 (opr: 1|shift|0) */
    {TOKEN_ADD, ADD_REG_OPCODE << 21, true},
    {TOKEN_ADDS, ADDS_REG_OPCODE << 21, true},
    {TOKEN_SUB, SUB_REG_OPCODE << 21, true},
    {TOKEN_SUBS, SUBS_REG_OPCODE << 21, true},

    /* Logical instructions, extra flag is the N bit */
    /* Has syntax: opc <Rd>, <Rn>, <Rm>{, <shift> #<imm> } */
    /* opcode: opc|0|101|0xxx (opr: 0|shift|N) */
    {TOKEN_AND, AND_OPCODE << 21, true},
    {TOKEN_BIC, BIC_OPCODE << 21, true},
    {TOKEN_ORR, ORR_OPCODE << 21, true},
    {TOKEN_ORN, ORN_OPCODE << 21, true}, /* N=1 */
    {TOKEN_EOR, EOR_OPCODE << 21, true},
    {TOKEN_EON, EON_OPCODE << 21, true}, /* N=1 */
    {TOKEN_ANDS, ANDS_OPCODE << 21, true},
    {TOKEN_BICS, BICS_OPCODE << 21, true}, /* N=1 */

    /* Multiply instructions, extra flag is the x bit */
    /* Has syntax: mul,mneg <Rd>, <Rn>, <Rm> */
    /* opcode: opc|1|101|1000 */
    {TOKEN_MADD, MADD_OPCODE << 21, true},
    {TOKEN_MSUB, MSUB_OPCODE << 21, true}, /* x=1 */
};

#define DP_INSTR_COUNT (sizeof(dp_instrs) / sizeof(dp_instrs[0]))

static dp_instr_info_t *find_dp_instr_info(token_mnemonic_t token, bool reg) {
  for (size_t i = 0; i < DP_INSTR_COUNT; i++) {
    if (dp_instrs[i].mnemonic == token && dp_instrs[i].reg == reg) {
      return &dp_instrs[i];
    }
  }
  return NULL;
}

static bool is_dp_instr_reg(instruction_IR_t *ps) {
  if (ps->operand_count < 2) {
    return false;
  }

  if (is_dp_mov_token(ps->mnemonic_tok))
    return false;
  if (is_dp_logical_token(ps->mnemonic_tok))
    return true;
  if (is_dp_mult_token(ps->mnemonic_tok))
    return true;
  if (is_dp_arith_token(ps->mnemonic_tok))
    return ps->operands[2].type == OPERAND_REGISTER;

  return false;
}

/* TODO: MOVE THIS ABSOLUTE NONSENSE FUNCTION TO PARSER SO THAT ALIAS
 * CONVERSION HAPPENS IN PARSER */
/* FIXME: WROTE THIS WHILE AIMING TO GET 592/592 TEST CASES PASS, NOT TO HANG
 * THIS CODE TO FUCKING LOUVRE MUSEUM */
static void convert_alias_dp_instrs(instruction_IR_t *ps) {
  switch (ps->mnemonic_tok) {
  case TOKEN_CMP:
  case TOKEN_CMN:
    if (ps->operand_count == 2) {
      *ps = (instruction_IR_t){
          (ps->mnemonic_tok == TOKEN_CMN) ? "adds" : "subs",
          (ps->mnemonic_tok == TOKEN_CMN) ? TOKEN_ADDS : TOKEN_SUBS,
          INSTR_DATA_PROCESSING,
          3,
          {(operand_t){OPERAND_REGISTER,
                       {.reg = {31, ps->operands[0].reg.is_64bit}}},
           ps->operands[0], ps->operands[1]}};
    } else if (ps->operand_count == 4) {
      *ps = (instruction_IR_t){
          (ps->mnemonic_tok == TOKEN_CMN) ? "adds" : "subs",
          (ps->mnemonic_tok == TOKEN_CMN) ? TOKEN_ADDS : TOKEN_SUBS,
          INSTR_DATA_PROCESSING,
          5,
          {(operand_t){OPERAND_REGISTER,
                       {.reg = {31, ps->operands[0].reg.is_64bit}}},
           ps->operands[0], ps->operands[1], ps->operands[2], ps->operands[3]}};
    }
    break;
  case TOKEN_NEG:
  case TOKEN_NEGS:
    if (ps->operand_count == 2) {
      *ps = (instruction_IR_t){
          (ps->mnemonic_tok == TOKEN_NEGS) ? "subs" : "sub",
          (ps->mnemonic_tok == TOKEN_NEGS) ? TOKEN_SUBS : TOKEN_SUB,
          INSTR_DATA_PROCESSING,
          3,
          {ps->operands[0],
           (operand_t){OPERAND_REGISTER,
                       {.reg = {31, ps->operands[0].reg.is_64bit}}},
           ps->operands[1]}};
    } else if (ps->operand_count == 4) {
      *ps = (instruction_IR_t){
          (ps->mnemonic_tok == TOKEN_NEGS) ? "subs" : "sub",
          (ps->mnemonic_tok == TOKEN_NEGS) ? TOKEN_SUBS : TOKEN_SUB,
          INSTR_DATA_PROCESSING,
          5,
          {ps->operands[0],
           (operand_t){OPERAND_REGISTER,
                       {.reg = {31, ps->operands[0].reg.is_64bit}}},
           ps->operands[1], ps->operands[2], ps->operands[3]}};
    }
    break;
  case TOKEN_TST:
    if (ps->operand_count == 2) {
      *ps = (instruction_IR_t){
          "ands",
          TOKEN_ANDS,
          INSTR_DATA_PROCESSING,
          3,
          {(operand_t){OPERAND_REGISTER,
                       {.reg = {31, ps->operands[0].reg.is_64bit}}},
           ps->operands[0], ps->operands[1]}};
    } else if (ps->operand_count == 4) {
      // add shifts to operands
      *ps = (instruction_IR_t){
          "ands",
          TOKEN_ANDS,
          INSTR_DATA_PROCESSING,
          5,
          {(operand_t){OPERAND_REGISTER,
                       {.reg = {31, ps->operands[0].reg.is_64bit}}},
           ps->operands[0], ps->operands[1], ps->operands[2], ps->operands[3]}};
    }
    break;
  case TOKEN_MVN:
    if (ps->operand_count == 2) {
      *ps = (instruction_IR_t){
          "orn",
          TOKEN_ORN,
          INSTR_DATA_PROCESSING,
          3,
          {ps->operands[0],
           (operand_t){OPERAND_REGISTER,
                       {.reg = {31, ps->operands[0].reg.is_64bit}}},
           ps->operands[1]}};
    } else if (ps->operand_count == 4) {
      // add shifts to operands
      *ps = (instruction_IR_t){
          "orn",
          TOKEN_ORN,
          INSTR_DATA_PROCESSING,
          5,
          {ps->operands[0],
           (operand_t){OPERAND_REGISTER,
                       {.reg = {31, ps->operands[0].reg.is_64bit}}},
           ps->operands[1], ps->operands[2], ps->operands[3]}};
    }
    break;
  case TOKEN_MOV:
    *ps = (instruction_IR_t){
        "orr",
        TOKEN_ORR,
        INSTR_DATA_PROCESSING,
        3,
        {ps->operands[0],
         (operand_t){OPERAND_REGISTER,
                     {.reg = {31, ps->operands[0].reg.is_64bit}}},
         ps->operands[1]}};
    break;

  case TOKEN_MUL:
    *ps = (instruction_IR_t){
        "madd",
        TOKEN_MADD,
        INSTR_DATA_PROCESSING,
        4,
        {ps->operands[0], ps->operands[1], ps->operands[2],
         (operand_t){OPERAND_REGISTER,
                     {.reg = {31, ps->operands[0].reg.is_64bit}}}}};
    break;

  case TOKEN_MNEG:
    *ps = (instruction_IR_t){
        "msub",
        TOKEN_MSUB,
        INSTR_DATA_PROCESSING,
        4,
        {ps->operands[0], ps->operands[1], ps->operands[2],
         (operand_t){OPERAND_REGISTER,
                     {.reg = {31, ps->operands[0].reg.is_64bit}}}}};
    break;
  default:
    break;
  }
}

/*                                                   v addr is not used */
u32 assemble_data_processing(instruction_IR_t *ps, u32 addr) {
  (void)addr;

  convert_alias_dp_instrs(ps);

  u32 instr = 0x0;

  bool is_instr_reg = is_dp_instr_reg(ps);
  dp_instr_info_t *dp_info = find_dp_instr_info(ps->mnemonic_tok, is_instr_reg);
  if (!dp_info) {
    fprintf(stderr,
            "Unsupported data processing instruction: token %d, reg_form=%d\n",
            ps->mnemonic_tok, is_instr_reg);
    exit(1);
  }

  /* TODO: operand count error handling? */

  instr = dp_info->opcode;

  operand_t rd = ps->operands[0];
  assert(rd.reg.reg_num <= MAX_REG_NUM);
  /* insert destination register bits (0-4) that is common to both imm and reg
   * type instructions */
  insert_bits_u32(&instr, 0, 4, rd.reg.reg_num);

  u8 sf = rd.reg.is_64bit; /* the sign flag */
  insert_bits_u32(&instr, 31, 31, sf);

  /* bits set until now:
         31 - sf
      if (instruction is immediate):
         30-23 - dp_info->opcode
      else
         30-21 - dp->info->opcode
        4-0 - rd
    */
  if (dp_info->reg) {
    /* rn is at 2nd operand, and rm is at 3rd operand */
    assert(ps->operand_count <= 5);
    operand_t rn = ps->operands[1];
    operand_t rm = ps->operands[2];

    assert(rn.reg.reg_num <= MAX_REG_NUM);
    insert_bits_u32(&instr, 5, 9, rn.reg.reg_num);

    assert(rm.reg.reg_num <= MAX_REG_NUM);
    insert_bits_u32(&instr, 16, 20, rm.reg.reg_num);

    switch (dp_info->mnemonic) {
    /* For arithmetic & logical instructions */
    case TOKEN_BIC:
    case TOKEN_BICS:
    case TOKEN_EON:
    case TOKEN_ORN:
      insert_bits_u32(&instr, 21, 21, 1);
    case TOKEN_ADD:
    case TOKEN_ADDS:
    case TOKEN_SUB:
    case TOKEN_SUBS:
    case TOKEN_AND:
    case TOKEN_ANDS:
    case TOKEN_EOR:
    case TOKEN_ORR:
      /* insert shift bits & operand */
      if (ps->operand_count == 5) {
        operand_t shift = ps->operands[3];
        operand_t shift_amt = ps->operands[4];
        if (shift.shift_type == ROR &&
            (ps->mnemonic_tok == TOKEN_ADD || ps->mnemonic_tok == TOKEN_ADDS ||
             ps->mnemonic_tok == TOKEN_SUB || ps->mnemonic_tok == TOKEN_SUBS)) {
          fprintf(stderr, "ROR type shift is only allowed in logical reg-type "
                          "instructions!\n");
          exit(1);
        }
        /* lsl = 0b00, lsr = 0b01, asr = 0b10, ror = 0b11 based on enum type */
        u32 shift_type_bits = (u32)shift.shift_type;

        assert(shift_type_bits <= 3); /* 0b11 */
        insert_bits_u32(&instr, 22, 23, shift_type_bits);

        u32 operand6 = shift_amt.immediate;
        if (rd.reg.is_64bit) {
          assert(operand6 <= 63); /* 0b111111 */
        } else {
          assert(operand6 <= 31); /* 0b011111 */
        }
        insert_bits_u32(&instr, 10, 15, operand6);
      }
      break;
    case TOKEN_MSUB:
      insert_bits_u32(&instr, 15, 15, 1);
    case TOKEN_MADD:
      assert(ps->operand_count == 4);
      operand_t ra = ps->operands[3];
      assert(ra.type == OPERAND_REGISTER);
      assert(ra.reg.reg_num <= MAX_REG_NUM);
      insert_bits_u32(&instr, 10, 14, ra.reg.reg_num);
      break;
    default:
      fprintf(
          stderr,
          "This mnemonic value is not a valid register dp instruction: %s\n",
          ps->mnemonic);
      break;
    }
  } else { /* instruction is immediate-typed */
    switch (dp_info->mnemonic) {
    case TOKEN_MOVK:
    case TOKEN_MOVZ:
    case TOKEN_MOVN: {
      assert(ps->operand_count == 2 || ps->operand_count == 4);
      u32 imm16 = ps->operands[1].immediate;
      assert(imm16 <= 0xffff);
      /* now immediate is in range 0 to 2^16 - 1 */
      insert_bits_u32(&instr, 5, 20, imm16);
      if (ps->operand_count == 4) {
        /* lsl and #imm operand exists */
        operand_t shift_amt = ps->operands[3];
        u32 hw = shift_amt.immediate / 16;
        if (!rd.reg.is_64bit) {
          /* for 32-bit move instruction hw can only take values 0b00
           * or 0b01 */
          assert(hw <= 1);
        }
        insert_bits_u32(&instr, 21, 22, hw);
      }
      break;
    }

    case TOKEN_ADD:
    case TOKEN_ADDS:
    case TOKEN_SUB:
    case TOKEN_SUBS: {
      assert(ps->operand_count == 3 || ps->operand_count == 5);
      operand_t rn = ps->operands[1];
      operand_t imm12 = ps->operands[2];

      assert(rn.reg.reg_num <= MAX_REG_NUM);
      insert_bits_u32(&instr, 5, 9, rn.reg.reg_num);

      assert(imm12.immediate <= 0xfff);
      insert_bits_u32(&instr, 10, 21, imm12.immediate);
      if (ps->operand_count == 5) {
        operand_t shift_amt = ps->operands[4];
        if (shift_amt.immediate == 12) {
          insert_bits_u32(&instr, 22, 22, 1);
        }
      }
      break;
    }
    default:
      fprintf(
          stderr,
          "This mnemonic value is not a valid immediate dp instruction: %s\n",
          ps->mnemonic);
      break;
    }
  }

  return instr;
}
