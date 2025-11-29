#ifndef LOAD_STORE
#define LOAD_STORE

#include "../emulate.h"

//Definition of bits in instruction
#define SINGLE_TRANSFER_BIT   31
#define SF_BIT                30
#define OP0_END               28
#define OP0_START             25
#define UNSIGNED_OFFSET_BIT   24
#define SIMM19_END            23
#define OPERATION_BIT         22
#define OFFSET_END            21
#define REGISTER_OFFSET_BIT   21
#define SIMM9_END             20
#define XM_END                20 
#define XM_START              16
#define SIMM9_START           12
#define I_BIT                 11
#define OFFSET_START          10
#define XN_END                 9
#define XN_START               5 
#define SIMM19_START           5
#define RT_END                 4
#define RT_START               0

//Possible values for OP0 for LOAD/STORE
#define OP0a 0x4 
#define OP0b 0x6
#define OP0c 0xc
#define OP0d 0xe

void execute_load_store(instruction instr);

#endif //LOAD_STORE
