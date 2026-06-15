# Intel8080emu

A cycle-accurate Intel 8080 CPU emulator that runs the original Space Invaders arcade ROM. Comes with a disassembler, hex dump utility, and the full opcode table mapped out.

## What's here

This is the core of what amounts to a working arcade machine emulator. Right now the emulated CPU can run through a program, execute instructions, and manipulate memory — but it's still a work in progress. A subset of the 256 opcodes are implemented in the execution path, enough to step through early Space Invaders ROM code.

### Files

- `m_to_assembly.cpp` — The main file. Contains the full opcode lookup table (all 256 Intel 8080 instructions), a disassembler that reads a binary and prints mnemonics, and an execution engine that interprets instructions against emulated CPU state.
- `invaders.h` — A C header containing the Space Invaders ROM data as a byte array. This gets compiled directly into the emulator so no external ROM file is needed.
- `my_hexdump.cpp` — A standalone hex dump utility. Takes a binary file and prints a formatted hex view. Useful for inspecting ROM contents.
- `invaders folder/` — The original Space Invaders ROM split into separate binary files. These correspond to the individual ROM chips on the arcade board: `invaders.e`, `invaders.f`, `invaders.g`, `invaders.h`, and a combined `invaders` file.
- `invaders` — The compiled emulator binary (generated from build).

## Building

No external dependencies. Just a C++ compiler.

```bash
g++ m_to_assembly.cpp -o invaders
```

Or if you want the hex dump tool:

```bash
g++ my_hexdump.cpp -o hexdump
```

## Running

```bash
./invaders
```

With no arguments, it prints a single instruction from the opcode table and exits. That's the default test behavior.

```bash
./invaders invaders_folder/invaders
```

With a ROM file path, it does two things:
1. Prints a hex dump of the entire file
2. Disassembles the file — walking through every byte, decoding it against the opcode table, and printing the instruction mnemonic and operands
3. Starts executing the ROM — loading it into emulated memory and running through instructions

## What's implemented

### Opcode table
All 256 Intel 8080 opcodes are mapped with their mnemonics and instruction sizes. The table covers everything from NOP and MOV to DAD, XCHG, and the RST instructions.

### Disassembler
The disassembler reads a binary file byte by byte, looks up each opcode in the table, and prints the instruction. It handles multi-byte instructions correctly — for a 3-byte instruction like `JMP adr`, it reads the next two bytes as the operand.

### Execution engine (partial)
The following instructions are wired up to actually modify CPU state:

| Category | Instructions |
|----------|-------------|
| Data transfer | MVI B, MVI M, MOV A,H, MOV M,A, LDAX D, LXI D, LXI H, LXI SP |
| Arithmetic | INR H, DCR B, INX H, INX D |
| Logical | CPI |
| Branching | JMP, JNZ, CALL, RET |
| Stack | PUSH B, PUSH D, PUSH H, PUSH PSW |

Most of the 78 unique Intel 8080 opcodes still need their execution handlers written. The opcode table is complete, so adding a new instruction means adding one `case` to the switch statement.

### CPU state
The emulated CPU tracks:
- Registers: A, B, C, D, E, H, L
- Stack pointer (SP) and program counter (PC)
- Flags: Zero, Sign, Parity, Carry, Auxiliary Carry
- Interrupt enable flag
- Full 64KB addressable memory array

## What doesn't work yet

The emulator can load a ROM and execute instructions, but it's not far enough along to actually run Space Invaders. Most opcodes still hit the `unimplemented_instruction()` handler and exit. The project is a solid foundation — the opcode mapping and basic execution framework are there — but the bulk of the instruction implementations still need to be written.

## How the emulator works

The Intel 8080 is an 8-bit CPU with a 16-bit address bus. It has seven 8-bit registers (A, B, C, D, E, H, L), a 16-bit stack pointer, and a 16-bit program counter. Instructions vary in size from 1 to 3 bytes.

The execution loop is straightforward:

```
pc = 0
while true:
    opcode = memory[pc]
    pc += instruction_size(opcode)
    execute(opcode)
```

The `execute` function uses a switch statement on the opcode byte. Each case modifies the CPU registers, memory, or flags as specified by the Intel 8080 instruction set manual.

## Why this exists

I wanted to understand how CPUs work at the level where software meets hardware. An emulator forces you to read the actual instruction set manual, implement every flag correctly, and handle edge cases like the difference between INR and INX (one sets flags, the other doesn't). It turns abstract computer architecture into something you can step through instruction by instruction.
