#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <array>

struct Opcode
{
    int opcode;
    std::string instruction;
    int size;
};

struct Cpu_frame
{
    u_int8_t a;
    u_int8_t b;
    u_int8_t c;
    u_int8_t d;
    u_int8_t e;
    u_int8_t h;
    u_int8_t l;
    u_int16_t sp;
    u_int16_t pc;
    std::bitset<8> flags;
    u_int8_t int_enable;
    std::vector<u_int8_t> stack;
};
struct Memory
{
    std::array<u_int8_t, 0xffff> Mblock;
};
enum flag_indicator
{
    Z,
    S,
    P,
    CY,
    AC,
};

std::unordered_map<int, Opcode> opcodeMap;

void print_instruction(int opnum)
{
    std::cout << opcodeMap[opnum].opcode << ' ' << opcodeMap[opnum].size << ' ' << opcodeMap[opnum].instruction << '\n';
}
std::vector<int16_t> getNextInstruction(std::ifstream &file, bool fullset = true);

void unimplemented_instruction(int opnum)
{
    std::cout << "This instruction is not implemented: ";
    print_instruction(opnum);
    // std::cout << '\b' << "\"\n";

    exit(0);
}
std::vector<int16_t> getNextInstructionArr(Memory &mem, int offset, bool fullset = true);

void executeNextInstruction(Memory &memory, Cpu_frame &cpu);
void jumpInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu);
void stackInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu);
void LXIInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu);
void updateFlags(u_int16_t answer, Cpu_frame &cpu);
void incrementInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu);
void moveInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu, Memory &mem);
void callandretInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu, Memory &memory);
void logicalInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu, Memory &memory);

int main(int argc, char *argv[])
{
    opcodeMap.insert({
        {0x00, {0x00, "NOP", 1}},
        {0x01, {0x01, "LXI B,D16", 3}},
        {0x02, {0x02, "STAX B", 1}},
        {0x03, {0x03, "INX B", 1}},
        {0x04, {0x04, "INR B", 1}},
        {0x05, {0x05, "DCR B", 1}},
        {0x06, {0x06, "MVI B, D8", 2}},
        {0x07, {0x07, "RLC", 1}},
        {0x08, {0x08, "-", 0}},
        {0x09, {0x09, "DAD B", 1}},
        {0x0a, {0x0a, "LDAX B", 1}},
        {0x0b, {0x0b, "DCX B", 1}},
        {0x0c, {0x0c, "INR C", 1}},
        {0x0d, {0x0d, "DCR C", 1}},
        {0x0e, {0x0e, "MVI C,D8", 2}},
        {0x0f, {0x0f, "RRC", 1}},
        {0x10, {0x10, "-", 0}},
        {0x11, {0x11, "LXI D,D16", 3}},
        {0x12, {0x12, "STAX D", 1}},
        {0x13, {0x13, "INX D", 1}},
        {0x14, {0x14, "INR D", 1}},
        {0x15, {0x15, "DCR D", 1}},
        {0x16, {0x16, "MVI D, D8", 2}},
        {0x17, {0x17, "RAL", 1}},
        {0x18, {0x18, "-", 0}},
        {0x19, {0x19, "DAD D", 1}},
        {0x1a, {0x1a, "LDAX D", 1}},
        {0x1b, {0x1b, "DCX D", 1}},
        {0x1c, {0x1c, "INR E", 1}},
        {0x1d, {0x1d, "DCR E", 1}},
        {0x1e, {0x1e, "MVI E,D8", 2}},
        {0x1f, {0x1f, "RAR", 1}},
        {0x20, {0x20, "-", 0}},
        {0x21, {0x21, "LXI H,D16", 3}},
        {0x22, {0x22, "SHLD adr", 3}}, //		(adr) <-L; (adr+1)<-H
        {0x23, {0x23, "INX H", 1}},    // HL <- HL + 1
        {0x24, {0x24, "INR H", 1}},    //	Z, S, P, AC	H <- H+1
        {0x25, {0x25, "DCR H", 1}},    // Z, S, P, AC	H <- H-1
        {0x26, {0x26, "MVI H,D8", 2}}, //		H <- byte 2
        {0x27, {0x27, "DAA", 1}},      // special
        {0x28, {0x28, "-", 0}},
        {0x29, {0x29, "DAD H", 1}},     // CY	HL = HL + HI
        {0x2a, {0x2a, "LHLD adr", 3}},  // L <- (adr); H<-(adr+1)
        {0x2b, {0x2b, "DCX H", 1}},     // HL = HL-1
        {0x2c, {0x2c, "INR L", 1}},     // Z, S, P, AC	L <- L+1
        {0x2d, {0x2d, "DCR L", 1}},     // Z, S, P, AC	L <- L-1
        {0x2e, {0x2e, "MVI L, D8", 2}}, // L <- byte 2
        {0x2f, {0x2f, "CMA", 1}},       // A <- !A
        {0x30, {0x30, "-", 0}},
        {0x31, {0x31, "LXI SP, D16", 3}}, //		SP.hi <- byte 3, SP.lo <- byte 2
        {0x32, {0x32, "STA adr", 3}},     //(adr) <- A
        {0x33, {0x33, "INX SP", 1}},      // SP = SP + 1
        {0x34, {0x34, "INR M", 1}},       // Z, S, P, AC	(HL) <- (HL)+1
        {0x35, {0x35, "DCR M", 1}},       // Z, S, P, AC	(HL) <- (HL)-1
        {0x36, {0x36, "MVI M,D8", 2}},    //		(HL) <- byte 2
        {0x37, {0x37, "STC", 1}},         // CY	CY = 1
        {0x38, {0x38, "-", 0}},
        {0x39, {0x39, "DAD SP", 1}},   //	CY	HL = HL + SP
        {0x3a, {0x3a, "LDA adr", 3}},  //	A <- (adr)
        {0x3b, {0x3b, "DCX SP", 1}},   //	SP = SP-1
        {0x3c, {0x3c, "INR A", 1}},    //	Z, S, P, AC	A <- A+1
        {0x3d, {0x3d, "DCR A", 1}},    // Z, S, P, AC	A <- A-1
        {0x3e, {0x3e, "MVI A,D8", 2}}, //		A <- byte 2
        {0x3f, {0x3f, "CMC", 1}},      //	CY	CY=!CY
        {0x40, {0x40, "MOV B,B", 1}},  //		B <- B
        {0x41, {0x41, "MOV B,C", 1}},  //	B <- C
        {0x42, {0x42, "MOV B,D", 1}},  //	B <- D
        {0x43, {0x43, "MOV B,E", 1}},  //	B <- E
        {0x44, {0x44, "MOV B,H", 1}},  //	B <- H
        {0x45, {0x45, "MOV B,L", 1}},  //	B <- L
        {0x46, {0x46, "MOV B,M", 1}},  //	B <- (HL)
        {0x47, {0x47, "MOV B,A", 1}},  //	B <- A
        {0x48, {0x48, "MOV C,B", 1}},  //	C <- B
        {0x49, {0x49, "MOV C,C", 1}},  //	C <- C
        {0x4a, {0x4a, "MOV C,D", 1}},  //	C <- D
        {0x4b, {0x4b, "MOV C,E", 1}},  //	C <- E
        {0x4c, {0x4c, "MOV C,H", 1}},  //	C <- H
        {0x4d, {0x4d, "MOV C,L", 1}},  //	C <- L
        {0x4e, {0x4e, "MOV C,M", 1}},  //	C <- (HL)
        {0x4f, {0x4f, "MOV C,A", 1}},  //	C <- A
        {0x50, {0x50, "MOV D,B", 1}},  //	D <- B
        {0x51, {0x51, "MOV D,C", 1}},  //	D <- C
        {0x52, {0x52, "MOV D,D", 1}},  //	D <- D
        {0x53, {0x53, "MOV D,E", 1}},  //	D <- E
        {0x54, {0x54, "MOV D,H", 1}},  //	D <- H
        {0x55, {0x55, "MOV D,L", 1}},  //	D <- L
        {0x56, {0x56, "MOV D,M", 1}},  //	D <- (HL)
        {0x57, {0x57, "MOV D,A", 1}},  //	D <- A
        {0x58, {0x58, "MOV E,B", 1}},  //	E <- B
        {0x59, {0x59, "MOV E,C", 1}},  //	E <- C
        {0x5a, {0x5a, "MOV E,D", 1}},  //	E <- D
        {0x5b, {0x5b, "MOV E,E", 1}},  //	E <- E
        {0x5c, {0x5c, "MOV E,H", 1}},  //	E <- H
        {0x5d, {0x5d, "MOV E,L", 1}},  //	E <- L
        {0x5e, {0x5e, "MOV E,M", 1}},  //	E <- (HL)
        {0x5f, {0x5f, "MOV E,A", 1}},  //	E <- A
        {0x60, {0x60, "MOV H,B", 1}},  //	H <- B
        {0x61, {0x61, "MOV H,C", 1}},  //	H <- C
        {0x62, {0x62, "MOV H,D", 1}},  //	H <- D
        {0x63, {0x63, "MOV H,E", 1}},  //	H <- E
        {0x64, {0x64, "MOV H,H", 1}},  //	H <- H
        {0x65, {0x65, "MOV H,L", 1}},  //	H <- L
        {0x66, {0x66, "MOV H,M", 1}},  //	H <- (HL)
        {0x67, {0x67, "MOV H,A", 1}},  //	H <- A
        {0x68, {0x68, "MOV L,B", 1}},  //	L <- B
        {0x69, {0x69, "MOV L,C", 1}},  //	L <- C
        {0x6a, {0x6a, "MOV L,D", 1}},  //	L <- D
        {0x6b, {0x6b, "MOV L,E", 1}},  //	L <- E
        {0x6c, {0x6c, "MOV L,H", 1}},  //	L <- H
        {0x6d, {0x6d, "MOV L,L", 1}},  //	L <- L
        {0x6e, {0x6e, "MOV L,M", 1}},  //	L <- (HL)
        {0x6f, {0x6f, "MOV L,A", 1}},  //	L <- A
        {0x70, {0x70, "MOV M,B", 1}},  //	(HL) <- B
        {0x71, {0x71, "MOV M,C", 1}},  //	(HL) <- C
        {0x72, {0x72, "MOV M,D", 1}},  //	(HL) <- D
        {0x73, {0x73, "MOV M,E", 1}},  //	(HL) <- E
        {0x74, {0x74, "MOV M,H", 1}},  //	(HL) <- H
        {0x75, {0x75, "MOV M,L", 1}},  //	(HL) <- L
        {0x76, {0x76, "HLT", 1}},      // special
        {0x77, {0x77, "MOV M,A", 1}},  //	(HL) <- A
        {0x78, {0x78, "MOV A,B", 1}},  //	A <- B
        {0x79, {0x79, "MOV A,C", 1}},  //	A <- C
        {0x7a, {0x7a, "MOV A,D", 1}},  //	A <- D
        {0x7b, {0x7b, "MOV A,E", 1}},  //	A <- E
        {0x7c, {0x7c, "MOV A,H", 1}},  //	A <- H
        {0x7d, {0x7d, "MOV A,L", 1}},  //	A <- L
        {0x7e, {0x7e, "MOV A,M", 1}},  //	A <- (HL)
        {0x7f, {0x7f, "MOV A,A", 1}},  //	A <- A
        {0x80, {0X80, "ADD B", 1}},    // Z, S, P, CY, AC	A <- A + B
        {0x81, {0X81, "ADD C", 1}},    // Z, S, P, CY, AC	A <- A + C
        {0x82, {0X82, "ADD D", 1}},    // Z, S, P, CY, AC	A <- A + D
        {0x83, {0X83, "ADD E", 1}},    // Z, S, P, CY, AC	A <- A + E
        {0x84, {0X84, "ADD H", 1}},    // Z, S, P, CY, AC	A <- A + H
        {0x85, {0X85, "ADD L", 1}},    // Z, S, P, CY, AC	A <- A + L
        {0x86, {0X86, "ADD M", 1}},    // Z, S, P, CY, AC	A <- A + (HL)
        {0x87, {0X87, "ADD A", 1}},    // Z, S, P, CY, AC	A <- A + A
        {0x88, {0X88, "ADC B", 1}},    // Z, S, P, CY, AC	A <- A + B + CY
        {0x89, {0X89, "ADC C", 1}},    // Z, S, P, CY, AC	A <- A + C + CY
        {0x8a, {0X8a, "ADC D", 1}},    // Z, S, P, CY, AC	A <- A + D + CY
        {0x8b, {0X8b, "ADC E", 1}},    // Z, S, P, CY, AC	A <- A + E + CY
        {0x8c, {0X8c, "ADC H", 1}},    // Z, S, P, CY, AC	A <- A + H + CY
        {0x8d, {0X8d, "ADC L", 1}},    // Z, S, P, CY, AC	A <- A + L + CY
        {0x8e, {0X8e, "ADC M", 1}},    // Z, S, P, CY, AC	A <- A + (HL) + CY
        {0x8f, {0X8f, "ADC A", 1}},    //	Z, S, P, CY, AC	A <- A + A + CY
        {0x90, {0X90, "SUB B", 1}},    // Z, S, P, CY, AC	A <- A - B
        {0x91, {0X91, "SUB C", 1}},    //	Z, S, P, CY, AC	A <- A - C
        {0x92, {0X92, "SUB D", 1}},    //	Z, S, P, CY, AC	A <- A + D
        {0x93, {0X93, "SUB E", 1}},    //	Z, S, P, CY, AC	A <- A - E
        {0x94, {0X94, "SUB H", 1}},    //	Z, S, P, CY, AC	A <- A + H
        {0x95, {0X95, "SUB L", 1}},    //	Z, S, P, CY, AC	A <- A - L
        {0x96, {0X96, "SUB M", 1}},    //	Z, S, P, CY, AC	A <- A + (HL)
        {0x97, {0X97, "SUB A", 1}},    //	Z, S, P, CY, AC	A <- A - A
        {0x98, {0X90, "SBB B", 1}},    //	Z, S, P, CY, AC	A <- A - B - CY
        {0x99, {0X990, "SBB C", 1}},   //	Z, S, P, CY, AC	A <- A - C - CY
        {0x9a, {0X9a, "SBB D", 1}},    //	Z, S, P, CY, AC	A <- A - D - CY
        {0x9b, {0X9b, "SBB E", 1}},    //	Z, S, P, CY, AC	A <- A - E - CY
        {0x9c, {0X9c, "SBB F", 1}},    //	Z, S, P, CY, AC	A <- A - H - CY
        {0x9d, {0X9d, "SBB G", 1}},    //	Z, S, P, CY, AC	A <- A - L - CY
        {0x9e, {0X9e, "SBB M", 1}},    //	Z, S, P, CY, AC	A <- A - (HL) - CY
        {0x9f, {0X9f, "SBB A", 1}},    //	Z, S, P, CY, AC	A <- A - A - CY
        {0xa0, {0Xa0, "ANA B", 1}},    //	Z, S, P, CY, AC	A <- A & B
        {0xa1, {0Xa1, "ANA C", 1}},    //	Z, S, P, CY, AC	A <- A & C
        {0xa2, {0Xa2, "ANA D", 1}},    //	Z, S, P, CY, AC	A <- A & D
        {0xa3, {0Xa3, "ANA E", 1}},    //	Z, S, P, CY, AC	A <- A & E
        {0xa4, {0Xa4, "ANA H", 1}},    //	Z, S, P, CY, AC	A <- A & H
        {0xa5, {0Xa5, "ANA L", 1}},    //	Z, S, P, CY, AC	A <- A & L
        {0xa6, {0Xa6, "ANA M", 1}},    //	Z, S, P, CY, AC	A <- A & (HL)
        {0xa7, {0Xa7, "ANA A", 1}},    //	Z, S, P, CY, AC	A <- A & A
        {0xa8, {0Xa8, "XRA B", 1}},    //	Z, S, P, CY, AC	A <- A ^ B
        {0xa9, {0Xa9, "XRA C", 1}},    //	Z, S, P, CY, AC	A <- A ^ C
        {0xaa, {0Xaa, "XRA D", 1}},    //	Z, S, P, CY, AC	A <- A ^ D
        {0xab, {0Xab, "XRA E", 1}},    //	Z, S, P, CY, AC	A <- A ^ E
        {0xac, {0Xac, "XRA H", 1}},    //	Z, S, P, CY, AC	A <- A ^ H
        {0xad, {0Xad, "XRA L", 1}},    //	Z, S, P, CY, AC	A <- A ^ L
        {0xae, {0Xae, "XRA M", 1}},    //	Z, S, P, CY, AC	A <- A ^ (HL)
        {0xaf, {0Xaf, "XRA A", 1}},    //	Z, S, P, CY, AC	A <- A ^ A
        {0xb0, {0Xb0, "ORA B", 1}},    //	Z, S, P, CY, AC	A <- A | B
        {0xb1, {0Xb1, "ORA C", 1}},    //	Z, S, P, CY, AC	A <- A | C
        {0xb2, {0Xb2, "ORA D", 1}},    //	Z, S, P, CY, AC	A <- A | D
        {0xb3, {0Xb3, "ORA E", 1}},    //	Z, S, P, CY, AC	A <- A | E
        {0xb4, {0Xb4, "ORA H", 1}},    //	Z, S, P, CY, AC	A <- A | H
        {0xb5, {0Xb5, "ORA L", 1}},    //	Z, S, P, CY, AC	A <- A | L
        {0xb6, {0Xb6, "ORA M", 1}},    //	Z, S, P, CY, AC	A <- A | (HL)
        {0xb7, {0Xb7, "ORA A", 1}},    //	Z, S, P, CY, AC	A <- A | A
        {0xb8, {0Xb8, "CMP B", 1}},    //	Z, S, P, CY, AC	A - B
        {0xb9, {0Xb9, "CMP B", 1}},    //	Z, S, P, CY, AC	A - C
        {0xba, {0Xba, "CMP B", 1}},    //	Z, S, P, CY, AC	A - D
        {0xbb, {0Xbb, "CMP B", 1}},    //	Z, S, P, CY, AC	A - E
        {0xbc, {0Xbc, "CMP B", 1}},    //	Z, S, P, CY, AC	A - H
        {0xbd, {0Xbd, "CMP B", 1}},    //	Z, S, P, CY, AC	A - L
        {0xbe, {0Xbe, "CMP B", 1}},    //	Z, S, P, CY, AC	A - (HL)
        {0xbf, {0Xbf, "CMP B", 1}},    //	Z, S, P, CY, AC	A - A
        {0xc0, {0xc0, "RNZ", 1}},      //	if NZ, RET
        {0xc1, {0xc1, "POP B", 1}},    //		C <- (sp); B <- (sp+1); sp <- sp+2
        {0xc2, {0xc2, "JNZ adr", 3}},  //	if NZ, PC <- adr
        {0xc3, {0xc3, "JMP adr", 3}},  //	PC <= adr
        {0xc4, {0xc4, "CNZ adr", 3}},  //	if NZ, CALL adr
        {0xc5, {0xc5, "PUSH B", 1}},   //	(sp-2)<-C; (sp-1)<-B; sp <- sp - 2
        {0xc6, {0xc6, "ADI D8", 2}},   //	Z, S, P, CY, AC	A <- A + byte
        {0xc7, {0xc6, "RST 0", 1}},    //		CALL $0
        {0xc8, {0xc8, "RZ", 1}},       //	if Z, RET
        {0xc9, {0xc9, "RET", 1}},      //	PC.lo <- (sp); PC.hi<-(sp+1); SP <- SP+2
        {0xca, {0xca, "JZ adr", 3}},   //		if Z, PC <- adr
        {0xcb, {0xcb, "-", 0}},
        {0xcc, {0xcc, "CZ adr", 3}},   //		if Z, CALL adr
        {0xcd, {0xcd, "CALL adr", 3}}, //	(SP-1)<-PC.hi;(SP-2)<-PC.lo;SP<-SP-2;PC=adr
        {0xce, {0xce, "ACI D8", 2}},   // Z, S, P, CY, AC	A <- A + data + CY
        {0xcf, {0xcf, "RST 1", 1}},    //	CALL $8
        {0xd0, {0xd0, "RNC", 1}},      // if NCY, RET
        {0xd1, {0xd1, "POP D", 1}},    //	E <- (sp); D <- (sp+1); sp <- sp+2
        {0xd2, {0xd2, "JNC adr", 3}},  //	if NCY, PC<-adr
        {0xd3, {0xd3, "OUT D8", 2}},   //	special
        {0xd4, {0xd5, "CNC adr", 3}},  //	if NCY, CALL adr
        {0xd5, {0xd5, "PUSH D", 1}},   //	(sp-2)<-E; (sp-1)<-D; sp <- sp - 2
        {0xd6, {0xd6, "SUI D8", 2}},   //	Z, S, P, CY, AC	A <- A - data
        {0xd7, {0xd7, "RST 2", 1}},    //	CALL $10
        {0xd8, {0xd8, "RC", 1}},       // if CY, RET
        {0xd9, {0xd9, "-", 0}},
        {0xda, {0xda, "JC adr", 3}}, //	if CY, PC<-adr
        {0xdb, {0xdb, "IN D8", 2}},  //	special
        {0xdc, {0xdc, "CC adr", 3}}, //	if CY, CALL adr
        {0xdd, {0xdd, "-", 0}},
        {0xde, {0xde, "SBI D8", 2}},   //	Z, S, P, CY, AC	A <- A - data - CY
        {0xdf, {0xdf, "RST 3", 1}},    //	CALL $18
        {0xe0, {0xe0, "RPO", 1}},      // if PO, RET
        {0xe1, {0xe1, "POP H", 1}},    //	L <- (sp); H <- (sp+1); sp <- sp+2
        {0xe2, {0xe2, "JPO adr", 3}},  //	if PO, PC <- adr
        {0xe3, {0xe3, "XTHL", 1}},     //	L <-> (SP); H <-> (SP+1)
        {0xe4, {0xe4, "CPO adr", 3}},  //	if PO, CALL adr
        {0xe5, {0xe5, "PUSH H", 1}},   //	(sp-2)<-L; (sp-1)<-H; sp <- sp - 2
        {0xe6, {0xe6, "ANI D8", 2}},   // Z, S, P, CY, AC	A <- A & data
        {0xe7, {0xe7, "RST 4", 1}},    //	CALL $20
        {0xe8, {0xe8, "RPE", 1}},      // if PE, RET
        {0xe9, {0xe9, "PCHL", 1}},     //	PC.hi <- H; PC.lo <- L
        {0xea, {0xea, "JPE adr", 3}},  //	if PE, PC <- adr
        {0xeb, {0xeb, "XCHG", 1}},     //	H <-> D; L <-> E
        {0xec, {0xec, "CPE adr", 3}},  //	if PE, CALL adr
        {0xed, {0xed, "-", 0}},        //
        {0xee, {0xee, "XRI D8", 2}},   // Z, S, P, CY, AC	A <- A ^ data
        {0xef, {0xef, "RST 5", 1}},    //	CALL $28
        {0xf0, {0xf0, "RP", 1}},       // if P, RET
        {0xf1, {0xf1, "POP PSW", 1}},  //	flags <- (sp); A <- (sp+1); sp <- sp+2
        {0xf2, {0xf2, "JP adr", 3}},   //	if P=1 PC <- adr
        {0xf3, {0xf3, "DI", 1}},       // special
        {0xf4, {0xf4, "CP adr", 3}},   //	if P, PC <- adr
        {0xf5, {0xf5, "PUSH PSW", 1}}, //	(sp-2)<-flags; (sp-1)<-A; sp <- sp - 2
        {0xf6, {0xf6, "ORI D8", 2}},   // Z, S, P, CY, AC	A <- A | data
        {0xf7, {0xf7, "RST 6", 1}},    //	CALL $30
        {0xf8, {0xf8, "RM", 1}},       // if M, RET
        {0xf9, {0xf9, "SPHL", 1}},     //	SP=HL
        {0xfa, {0xfa, "JM adr", 3}},   //	if M, PC <- adr
        {0xfb, {0xfb, "EI", 1}},       // special
        {0xfc, {0xfc, "CM adr", 3}},   //	if M, CALL adr
        {0xfd, {0xfd, "	-	", 0}},
        {0xfe, {0xfe, "CPI D8", 2}}, // Z, S, P, CY, AC	A - data
        {0xff, {0xff, "RST 7", 1}}   //	CALL $38
    });

    print_instruction(0x02);

    if (argc > 1)
    {
        // std::cout << argv[0] << '\n';

        std::ifstream file_ref{argv[1]};

        if (!(static_cast<bool>(file_ref)))
        {
            std::cout << '\"' << argv[1] << '\"' << "  is an invalid filename\n";
            return 1;
        }

        std::cout << std::hex << std::setw(8) << std::right << std::setfill('0');
        std::cout << 0 << "  ";

        while (file_ref)
        {
            // file_ref.seekg(i,std::ios::beg);
            // std::cout <<'\n' << '[' << file_ref.tellg() <<']' << '\n';

            int16_t block1{getNextInstruction(file_ref, false)[0]};
            int16_t block2{getNextInstruction(file_ref, false)[0]};

            std::cout << std::hex << std::setw(2) << std::right << std::setfill('0');
            std::cout << static_cast<int>(block2);

            std::cout << std::hex << std::setw(2) << std::right << std::setfill('0');
            std::cout << static_cast<int>(block1) << ' ';

            if ((file_ref.tellg() % 0x10) == 0)
            {
                std::cout << '\n';
                std::cout << std::hex << std::setw(8) << std::right << std::setfill('0');

                std::cout << file_ref.tellg() << "  ";
            }
        }
        std::cout << '\n';

        file_ref.close();
        file_ref.open(argv[1]);

        while (file_ref)
        {

            int16_t block2;

            std::cout << std::hex << std::setw(8) << std::right << std::setfill('0');
            std::cout << file_ref.tellg() << "  ";

            std::vector<int16_t> array_of_instruction{getNextInstruction(file_ref)};

            int16_t block1{array_of_instruction[0]};

            std::cout << std::hex << std::setw(2) << std::right << std::setfill('0');
            std::cout << opcodeMap[block1].opcode << ' ' << opcodeMap[block1].size << ' ' << opcodeMap[block1].instruction << ' ';

            for (int i = 1; i < opcodeMap[block1].size; i++)
            {
                block2 = array_of_instruction[i];

                std::cout << std::hex << std::setw(2) << std::right << std::setfill('0');
                std::cout << static_cast<int>(block2);
            }

            std::cout << '\n';
        }
        std::cout << '\n';

        file_ref.close();
        file_ref.open(argv[1]);
        Cpu_frame file_cpu;
        Memory file_memory;

        // Populate memory
        int16_t block1;
        while (file_ref)
        {
            file_ref.read(reinterpret_cast<char *>(&block1), 1);
            file_memory.Mblock[static_cast<int>(file_ref.tellg()) - 1] = block1;
        }

        while (true)
        {
            executeNextInstruction(file_memory, file_cpu);
        }

        return 0;
    }
}

std::vector<int16_t> getNextInstruction(std::ifstream &file, bool fullset)
{
    int16_t block1;
    file.read(reinterpret_cast<char *>(&block1), 1);

    if (!fullset)
        return {block1};

    std::vector<int16_t> instruction_array;

    instruction_array.push_back(block1);

    int16_t block2;

    for (int i = 1; i < opcodeMap[block1].size; i++)
    {
        file.read(reinterpret_cast<char *>(&block2), 1);
        instruction_array.push_back(block2);
    }

    return instruction_array;
}

std::vector<int16_t> getNextInstructionArr(Memory &mem, int offset, bool fullset)
{
    int16_t block1{mem.Mblock[offset]};
    // file.read(reinterpret_cast<char *>(&block1), 1);

    if (!fullset)
        return {block1};

    std::vector<int16_t> instruction_array;

    instruction_array.push_back(block1);

    int16_t block2;

    for (int i = 1; i < opcodeMap[block1].size; i++)
    {
        block2 = mem.Mblock[offset + i];
        instruction_array.push_back(block2);
    }

    return instruction_array;
}

void executeNextInstruction(Memory &memory, Cpu_frame &cpu)
{
    std::vector<int16_t> array_of_instruction{getNextInstructionArr(memory, cpu.pc)};

    // std::cout << cpu.pc << ' ' << array_of_instruction[0] << ' ' << array_of_instruction.size() << '\n';

    cpu.pc += array_of_instruction.size();

    switch (array_of_instruction[0])
    {
    case 0x0:
        break;
    case 0xc3:
        jumpInstruction(array_of_instruction, cpu);
        break;
    case 0xf5:
        stackInstruction(array_of_instruction, cpu);
        break;
    case 0xc5:
        stackInstruction(array_of_instruction, cpu);
        break;
    case 0xd5:
        stackInstruction(array_of_instruction, cpu);
        break;
    case 0xe5:
        stackInstruction(array_of_instruction, cpu);
        break;
    case 0x31:
        LXIInstruction(array_of_instruction, cpu);
        break;
    case 0x24:
        incrementInstruction(array_of_instruction, cpu);
        break;
    case 0x6:
        moveInstruction(array_of_instruction, cpu, memory);
        break;
    case 0xcd:
        callandretInstruction(array_of_instruction, cpu, memory);
        break;
    case 0x11:
        LXIInstruction(array_of_instruction, cpu);
        break;
    case 0x21:
        LXIInstruction(array_of_instruction, cpu);
        break;
    case 0x1a:
        moveInstruction(array_of_instruction, cpu, memory);
        break;
    case 0x77:
        moveInstruction(array_of_instruction, cpu, memory);
        break;
    case 0x23:
        incrementInstruction(array_of_instruction, cpu);
        break;
    case 0x13:
        incrementInstruction(array_of_instruction, cpu);
        break;
    case 0x5:
        incrementInstruction(array_of_instruction, cpu);
        break;
    case 0xc2:
        jumpInstruction(array_of_instruction, cpu);
        break;
    case 0xc9:
        callandretInstruction(array_of_instruction, cpu, memory);
        break;
    case 0x36:
        moveInstruction(array_of_instruction, cpu, memory);
        break;
    case 0x7c:
        moveInstruction(array_of_instruction, cpu, memory);
        break;
    case 0xfe:
        logicalInstruction(array_of_instruction, cpu, memory);
        break;

    default:
        unimplemented_instruction(array_of_instruction[0]);
        break;
    }
    // file.seekg(cpu.pc, std::ios::beg);
}

void jumpInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu)
{
    switch (instructions[0])
    {
    case 0xc3:
        cpu.pc = (instructions[2] << 8) | instructions[1];
        break;
    case 0xc2:
        if (!(cpu.flags[Z]))
        {
            cpu.pc = (instructions[2] << 8) | instructions[1];
        }
        break;

    default:
        std::cout << instructions[0] << '\n';
        break;
    }
}
void stackInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu)
{
    switch (instructions[0])
    {
    case 0x31:
        cpu.sp = (instructions[2] << 8) | instructions[1];
        break;

    default:
        std::cout << instructions[0] << '\n';
        break;
    }
    // cpu.pc++;
}
void LXIInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu)
{
    switch (instructions[0])
    {
    case 0x31:
        cpu.sp = (instructions[2] << 8) | instructions[1];
        break;
    case 0x11:
        cpu.d = instructions[2];
        cpu.e = instructions[1];
        break;
    case 0x21:
        cpu.h = instructions[2];
        cpu.l = instructions[1];
        break;

    default:
        std::cout << instructions[0] << '\n';
        break;
    }
    // cpu.pc++;
}
void incrementInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu)
{
    switch (instructions[0])
    {
    case 0x24:
    {
        u_int16_t temp = cpu.h + 1;
        cpu.h = temp & 0xff;
        bool carryflag = cpu.flags[CY];
        updateFlags(temp, cpu);
        cpu.flags[CY] = carryflag;
    }
    break;
    case 0x23:
    {
        u_int16_t temp = (((static_cast<u_int16_t>(cpu.h) & 0xff) << 8)) | static_cast<u_int16_t>(cpu.l) + 1;
        cpu.h = (temp >> 8) & 0xff;
        cpu.l = temp & 0xff;

        // std::cout << (int)cpu.h << ' ' <<(int)cpu.l << '\n';
    }
    break;
    case 0x13:
    {
        u_int16_t temp = (((static_cast<u_int16_t>(cpu.d) & 0xff) << 8)) | static_cast<u_int16_t>(cpu.e) + 1;
        cpu.d = (temp >> 8) & 0xff;
        cpu.e = temp & 0xff;

        // std::cout << (int)cpu.d << ' ' <<(int)cpu.e << '\n';
    }
    break;
    case 0x5:
    {
        u_int16_t temp = cpu.b - 1;
        cpu.b = temp & 0xff;
        bool carryflag = cpu.flags[CY];
        updateFlags(temp, cpu);
        cpu.flags[CY] = carryflag;
    }
    break;
    default:
        std::cout << instructions[0] << '\n';
        break;
    }
    // cpu.pc++;
}
void moveInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu, Memory &mem)
{
    switch (instructions[0])
    {
    case 0x6:
        cpu.b = instructions[1];
        break;
    case 0x1a:
    {
        u_int16_t temp = (((static_cast<u_int16_t>(cpu.d) & 0xff) << 8)) | static_cast<u_int16_t>(cpu.e);
        cpu.a = mem.Mblock[temp];
        // std::cout << (int)cpu.a << ' ' << (int)cpu.d << ' ' << (int)cpu.e << ' ' << temp << '\n';
    }
    break;
    case 0x77:
    {
        u_int16_t temp = (((static_cast<u_int16_t>(cpu.h) & 0xff) << 8)) | static_cast<u_int16_t>(cpu.l);
        mem.Mblock[temp] = cpu.a;
        // std::cout << (int)mem.Mblock[temp] << '\n';
    }
    break;
    case 0x36:
    {
        u_int16_t temp = (((static_cast<u_int16_t>(cpu.h) & 0xff) << 8)) | static_cast<u_int16_t>(cpu.l);
        mem.Mblock[temp] = instructions[1];
        // std::cout << (int)mem.Mblock[temp] << '\n';
    }
    break;
    case 0x7c:
        cpu.a = cpu.h;
        break;

    default:
        std::cout << instructions[0] << '\n';
        break;
    }
    // cpu.pc++;
}
void callandretInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu, Memory &memory)
{
    switch (instructions[0])
    {
    case 0xcd:
        memory.Mblock[cpu.sp - 1] = (cpu.pc >> 8) & 0xff;
        memory.Mblock[cpu.sp - 2] = cpu.pc & 0xff;
        cpu.sp = cpu.sp - 2;
        cpu.pc = (instructions[2] << 8) | instructions[1];
        break;
    case 0xc9:
        cpu.pc = ((static_cast<u_int16_t>(memory.Mblock[cpu.sp + 1]) & 0xff) << 8) | (static_cast<u_int16_t>(memory.Mblock[cpu.sp]) & 0xff);
        cpu.sp = cpu.sp - 2;
        break;

    default:
        std::cout << instructions[0] << '\n';
        break;
    }
}

void logicalInstruction(std::vector<int16_t> instructions, Cpu_frame &cpu, Memory &memory)
{
    switch (instructions[0])
    {

    case 0xfe:
    {
        u_int16_t temp = cpu.a - instructions[1];
        updateFlags(temp, cpu);
    }
    break;

    default:
        std::cout << instructions[0] << '\n';
        break;
    }
}

void updateFlags(u_int16_t answer, Cpu_frame &cpu)
{
    cpu.flags[Z] = ((answer & 0xff) == 0);
    cpu.flags[S] = ((answer & 0x08) != 0);
    cpu.flags[P] = __builtin_parity(answer);
    cpu.flags[CY] = (answer > 0xff);
    cpu.flags[AC] = false;
}
