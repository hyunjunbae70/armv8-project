# ARMv8 Emulator & Assembler

Team project building a complete AArch64 toolchain - an assembler that translates assembly to binary, and an emulator that simulates instruction execution.

## Components

- **Emulator**: Simulates ARMv8 binary instructions with full machine state tracking (registers, memory, PC, flags)
- **Assembler**: Translates ARMv8 assembly source into executable binaries

Developed by a team of 4. Includes extension projects (LED control program for Raspberry Pi, IQ Fit puzzle game).

## Features

### Emulator
- Emulates a subset of ARMv8/AArch64 instructions
- Simulates complete machine state including 2MB of main memory
- Supports instruction execution and step-by-step debugging
- Outputs machine state to standard output or specified file

### Assembler
- Assembles ARMv8 assembly source files into executable binary
- Supports multiple instruction types including:
  - Data processing instructions
  - Branch instructions
  - Load/store instructions
  - Immediate and register-based operations

## Getting Started

### Prerequisites
- GCC compiler
- Make build system
- A Linux/Unix-like environment (tested on Ubuntu/WSL)

### Building the Project

Build both components:
```bash
make
```

Or build individually:
```bash
make emulator
make assembler
```

Clean build artifacts:
```bash
make clean
```

Rebuild from scratch:
```bash
make rebuild
```

## Usage

### Emulator

Run the emulator on a compiled ARMv8 object file:

```bash
./emulator/emulate [file_in] [file_out]
```

- `file_in`: ARMv8 object file to emulate
- `file_out`: (Optional) Output file for machine state. If omitted, prints to stdout

**Example:**
```bash
./emulator/emulate program.o output.txt
```

### Assembler

Assemble an ARMv8 assembly source file:

```bash
./assembler/assemble [input] [output]
```

- `input`: ARMv8 assembly source file (.s)
- `output`: Output binary/object file

**Example:**
```bash
./assembler/assemble led_blink.s led_blink.o
```

## Project Structure

```
.
├── src/
│   ├── assembler/          # Assembler source code
│   │   ├── assemble*.c     # Instruction assembly modules
│   │   ├── parser.c        # Assembly parser
│   │   └── Makefile
│   ├── emulator/           # Emulator source code
│   │   ├── execute/        # Instruction execution modules
│   │   ├── machine.c       # Machine state management
│   │   └── Makefile
│   ├── utils/              # Shared utilities
│   │   ├── bits_utils.c    # Bit manipulation utilities
│   │   ├── hashmap.c       # Hash map data structure
│   │   └── vector.c        # Dynamic array implementation
│   └── defs.h              # Shared definitions
├── extension/              # Extension projects
│   └── iq_fit.c           # Puzzle game (C graphics library)
├── led_blink.s            # LED control program (A64 assembly)
└── README.md
```

## Extension Projects

### LED Blink Program (`led_blink.s`)

An A64 assembly program that controls an LED on a Raspberry Pi board. This program demonstrates:
- GPIO register manipulation
- Hardware interaction through memory-mapped I/O
- Timing control using delay loops
- Real-world application of the assembler

**Usage:**
1. Assemble the program: `./assembler/assemble led_blink.s led_blink.o`
2. Load and execute on Raspberry Pi hardware

### IQ Fit Puzzle Game (`extension/iq_fit.c`)

A C-language puzzle game developed using a graphics library. This extension demonstrates:
- Advanced problem-solving algorithms (backtracking)
- Game logic implementation
- User interface design
- Integration of C code with the ARMv8 toolchain

The game presents players with puzzle pieces to fit onto a board, with features including:
- Multiple solution generation
- Piece rotation support
- Interactive gameplay with HP system
- Random puzzle generation

**Note:** This extension project was nominated for best extension in the course.

## Technical Details

### Supported Instructions

The emulator and assembler support a subset of ARMv8 instructions including:

- **Data Processing**: Arithmetic and logical operations
- **Branch Instructions**: Conditional and unconditional branches
- **Load/Store**: Memory access operations
- **Immediate Operations**: Operations with immediate values
- **Register Operations**: Operations between registers

### Machine State

The emulator maintains:
- 2MB of main memory
- Complete register file (32 general-purpose registers)
- Program counter (PC)
- Processor status flags

