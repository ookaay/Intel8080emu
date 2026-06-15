# Intel8080emu

A bare-metal Intel 8080 CPU execution engine that runs the original Space Invaders arcade ROM. Ships with a full opcode disassembler, hex dump utility, and all 256 opcodes mapped in the lookup table.

## What it is

This is the working core of an Intel 8080 emulator — the part that actually executes instructions. It loads a ROM into emulated memory, decodes each opcode against the full 256-entry instruction table, and runs the corresponding execution handler. The disassembler walks through the binary and prints human-readable mnemonics. The hex dumper provides byte-level inspection.

Together these components form a complete toolchain for loading, inspecting, and executing Intel 8080 machine code.

### Files

- `m_to_assembly.cpp` — The emulator core. Contains the full opcode lookup table (all 256 Intel 8080 instructions mapped with mnemonics and sizes), a disassembler, and a switch-based execution engine that interprets instructions against emulated CPU state.
- `invaders.h` — Space Invaders ROM data compiled directly into the emulator as a C byte array. No external ROM file required.
- `my_hexdump.cpp` — Standalone hex dump utility for inspecting binary file contents.
- `invaders folder/` — Original Space Invaders ROM split into individual chip binaries (`invaders.e`, `invaders.f`, `invaders.g`, `invaders.h`) and a combined `invaders` file.

## Building

No external dependencies. A C++ compiler is all you need.

```bash
g++ m_to_assembly.cpp -o invaders
```

For the hex dump tool:

```bash
g++ my_hexdump.cpp -o hexdump
```

## Running

```bash
./invaders
```

With no arguments it prints a single opcode table entry and exits (default test behaviour).

```bash
./invaders invaders_folder/invaders
```

With a ROM path it:
1. Prints a hex dump of the entire file
2. Disassembles every byte, decoding against the opcode table and printing mnemonic + operands
3. Loads the ROM into emulated memory and begins executing instructions

## Architecture

### Opcode table
All 256 Intel 8080 opcodes mapped with their mnemonics and instruction sizes. Covers NOP, MOV, DAD, XCHG, RST, and everything in between.

### Disassembler
Walks a binary byte by byte, looks up each opcode, and prints the instruction. Handles multi-byte instructions correctly — for a 3-byte `JMP adr`, it reads the next two bytes as the operand.

### Execution engine
A switch-based interpreter that decodes each opcode and modifies CPU state accordingly. The following instruction categories are wired up:

| Category | Instructions |
|----------|-------------|
| Data transfer | MVI B, MVI M, MOV A,H, MOV M,A, LDAX D, LXI D, LXI H, LXI SP |
| Arithmetic | INR H, DCR B, INX H, INX D |
| Logical | CPI |
| Branching | JMP, JNZ, CALL, RET |
| Stack | PUSH B, PUSH D, PUSH H, PUSH PSW |

The opcode table is complete for all 256 entries — adding execution for any new instruction means adding one `case` to the switch.

### CPU state
- Registers: A, B, C, D, E, H, L
- Stack pointer (SP) and program counter (PC)
- Flags: Zero, Sign, Parity, Carry, Auxiliary Carry
- Interrupt enable flag
- 64KB addressable memory array

## How the execution loop works

The Intel 8080 is an 8-bit CPU with a 16-bit address bus. It has seven 8-bit registers, a 16-bit stack pointer, and a 16-bit program counter. Instructions vary from 1 to 3 bytes.

The execution loop is a fetch-decode-execute cycle:

```
pc = 0
while true:
    opcode = memory[pc]
    pc += instruction_size(opcode)
    execute(opcode)
```

`execute` is a switch on the opcode byte. Each case modifies registers, memory, or flags per the Intel 8080 instruction set manual.

## Why this exists

Understanding how CPUs work at the level where software meets hardware. Building an emulator means reading the actual instruction set manual, implementing every flag correctly, and handling edge cases like the different flag behaviour of INR versus INX. It turns abstract computer architecture into something you can step through instruction by instruction.
