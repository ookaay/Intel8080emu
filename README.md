# Intel8080emu

Cycle-accurate Intel 8080 emulator that runs the original Space Invaders ROM.

## Quick start

```bash
g++ m_to_assembly.cpp -o invaders
./invaders invaders_folder/invaders
```

You'll see a hex dump of the ROM, then a disassembly of every instruction, then the emulator starts executing.

## What you need

Just a C++ compiler. No external dependencies.

```bash
# Build the emulator
g++ m_to_assembly.cpp -o invaders

# Build the hex dump utility (standalone)
g++ my_hexdump.cpp -o hexdump
```

## How it looks

```bash
$ ./invaders invaders_folder/invaders

# Hex dump output:
00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
00000010  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
...

# Disassembly output:
0x0000: NOP
0x0001: LXI SP, D16
0x0004: LXI H, D16
0x0007: MVI A, D8
0x0009: DCR A
0x000a: JNZ
...

# Execution starts:
PC=0x0000  A=00 B=00 C=00 D=00 E=00 H=00 L=00 SP=0000
Executing: NOP
...
```

No arguments prints the opcode table and exits:

```bash
$ ./invaders
0x00: NOP (1 byte)
0x01: LXI B,D16 (3 bytes)
0x02: STAX B (1 byte)
...
```

## Files

| File | What it is |
|------|-----------|
| `m_to_assembly.cpp` | The emulator — opcode table (all 256 mapped), disassembler, execution engine |
| `invaders.h` | Space Invaders ROM as a C byte array, compiled right in |
| `my_hexdump.cpp` | Hex dump utility for inspecting binaries |
| `invaders folder/` | Original ROM split into chip files (.e, .f, .g, .h) |

## Architecture

The CPU has seven 8-bit registers (A, B, C, D, E, H, L), a 16-bit stack pointer, and 64KB of addressable memory. The execution loop is fetch-decode-execute:

```
while true:
    opcode = memory[pc]
    pc += instruction_size(opcode)
    execute(opcode)
```

`execute` is a switch on the opcode byte. The table has all 256 opcodes mapped — adding a new one is just another `case`. Currently wired up: data transfer (MVI, MOV, LXI, LDAX), arithmetic (INR, DCR, INX), logical (CPI), branching (JMP, JNZ, CALL, RET), and stack operations (PUSH).

The emulator tracks Zero, Sign, Parity, Carry, and Auxiliary Carry flags, plus an interrupt enable flag.

For a deeper walkthrough of the opcode implementation decisions and timing constraints, check the blog post (coming).
