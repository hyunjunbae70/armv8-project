# armv8-project
armv8 emulator &amp; assembler

Emulator
Files can be found in src/emulator/*

Emulates a subset of ARMv8 instructions, given an ARMv8 object file. Prints out the machine state (2MB of main memory) to standard output or to the given output file.

Can be used as:

./emulate [file_in] [file_out (OPTIONAL)]
If no file_out is provided, prints the machine state to stdout.

Assembler
Files can be found in src/assembler/*

Assembles a given ARMv8 (subset) assembly file.

Can be used as:

./assemble [input] [output]
