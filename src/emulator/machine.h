#ifndef MACHINE
#define MACHINE

#include "../defs.h"
#include "emulate.h"

void run_machine(machine_t *machine);
void shutdown_machine(machine_t *machine, FILE *out_stream);

void machine_load_program(machine_t *machine, const char *prg);

#endif /* MACHINE */
