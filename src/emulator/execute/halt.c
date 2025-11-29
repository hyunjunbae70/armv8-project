#include "halt.h"

bool halt_instr(instruction instr) { return instr == TERMINATE_OP; }
