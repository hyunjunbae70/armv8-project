#include "branches.h"
#include "../../utils/bits_utils.h"
#include <stdbool.h>
#include <stdlib.h>

#define BRANCH_AL_ENCODING 0xe
#define BRANCH_EQ_ENCODING 0x0
#define BRANCH_GQ_ENCODING 0xa
#define BRANCH_GT_ENCODING 0xc
#define BRANCH_LE_ENCODING 0xd
#define BRANCH_LT_ENCODING 0xb
#define BRANCH_NE_ENCODING 0x1

#define BRANCH_UNCONDITIONAL 0x0
#define BRANCH_REG 0x3
#define BRANCH_CONDITIONAL 0x1

typedef bool (*b_cond_f)(void);

static bool check_eq(void) { return machine.pstate.Z; }
static bool check_ne(void) { return !machine.pstate.Z; }
static bool check_gq(void) { return machine.pstate.N == machine.pstate.V; }
static bool check_lt(void) { return machine.pstate.N != machine.pstate.V; }
static bool check_gt(void) {
  return machine.pstate.Z == 0 && machine.pstate.N == machine.pstate.V;
}
static bool check_le(void) {
  return machine.pstate.Z || machine.pstate.N != machine.pstate.V;
}
static bool check_al(void) { return true; }

bool branch_instr(instruction instr) {
  /*
   * uncondtional: 0 00101 simm26
   * register: 1101011 0 0 00 11111 0000 0 0 xn 00000
   * conditional: 0101010 0 simm19 0 cond
   */
  u8 branch_type = extract_bits_u32(instr, 30, 31);
  switch (branch_type) {
  case BRANCH_UNCONDITIONAL: { /* unconditional branch with signed immediate */
    i32 simm26 = (i32)sign_extend(extract_bits_u32(instr, 0, 25), 26);
    machine.PC += (simm26) * sizeof(instruction);
    break;
  }
  case BRANCH_REG: { /* unconditional register branch */
    u8 xn = extract_bits_u32(instr, 5, 9);
    machine.PC = machine.regs[xn];
    break;
  }
  case BRANCH_CONDITIONAL: { /* conditional branch with signed immediate */
    i32 simm19 = sign_extend(extract_bits_u32(instr, 5, 23), 19);
    u8 cond = extract_bits_u32(instr, 0, 3);

    b_cond_f condition_funcs[] = {
        [BRANCH_EQ_ENCODING] = check_eq, [BRANCH_NE_ENCODING] = check_ne,
        [BRANCH_GQ_ENCODING] = check_gq, [BRANCH_LT_ENCODING] = check_lt,
        [BRANCH_GT_ENCODING] = check_gt, [BRANCH_LE_ENCODING] = check_le,
        [BRANCH_AL_ENCODING] = check_al};

    if (cond >= sizeof(condition_funcs) / sizeof(condition_funcs[0]) ||
        condition_funcs[cond] == NULL) {
      fprintf(stderr, "NOT A VALID BRANCH CONDITION ENCODING.\n");
      return false;
    }

    if (condition_funcs[cond]()) {
      machine.PC += simm19 * sizeof(instruction);
    }
  }
  }
  return true;
}
