#include <fstream>

#include "chip8.hpp"

Chip8::Chip8() : 
	SoundTimer(0), DelayTimer(0),
	pc(0x200), SP(0), I(0), opcode(0)
{
	for (int i = 0; i < 80; ++i) {
		Mem[i] = FontSet[i];
	}

	// Set up function pointer table
	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1nnn;
	table[0x2] = &Chip8::OP_2nnn;
	table[0x3] = &Chip8::OP_3xkk;
	table[0x4] = &Chip8::OP_4xkk;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xkk;
	table[0x7] = &Chip8::OP_7xkk;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxkk;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	tableF[0x07] = &Chip8::OP_Fx07;
	tableF[0x0A] = &Chip8::OP_Fx0A;
	tableF[0x15] = &Chip8::OP_Fx15;
	tableF[0x18] = &Chip8::OP_Fx18;
	tableF[0x1E] = &Chip8::OP_Fx1E;
	tableF[0x29] = &Chip8::OP_Fx29;
	tableF[0x33] = &Chip8::OP_Fx33;
	tableF[0x55] = &Chip8::OP_Fx55;
	tableF[0x65] = &Chip8::OP_Fx65;
}

void
Chip8::loadRom(const char *filename) {
	unsigned startPos = 0x200;
	
	for (std::ifstream rom(filename, std::ios::binary); rom.good() ;)
		Mem[startPos++ & 0xFFF] = rom.get();
}

void
Chip8::cycle() {

	// Fetch opcode
	opcode = Mem[pc] << 8 | Mem[pc + 1];

	// Increment pc
	pc += 2;

	// Set aliases
	x = (opcode & 0x0F00) >> 8;
	y = (opcode & 0x00F0) >> 4;
	n = opcode & 0x000F;
	nnn = opcode & 0x0FFF;
	kk = opcode & 0x00FF;

	// Decode and execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();
	
	// Update  timers
	if (DelayTimer > 0) --DelayTimer;
	if (SoundTimer > 0) --SoundTimer;
}

void
Chip8::Table0() {
	((*this).*(table0[opcode & 0x000Fu]))();
}

void
Chip8::Table8() {
	((*this).*(table8[opcode & 0x000Fu]))();
}

void
Chip8::TableE() {
	((*this).*(tableE[opcode & 0x000Fu]))();
}

void
Chip8::TableF() {
	((*this).*(tableF[opcode & 0x00FFu]))();
}

void
Chip8::OP_NULL() {  }

void
Chip8::OP_00E0() {
	for (auto &p : Display) p = 0;
}

void
Chip8::OP_00EE() {
	pc = Stack[SP--];
}

void
Chip8::OP_1nnn() {
	pc = nnn;
}

void
Chip8::OP_2nnn() {
	// Save pc for ret call
	Stack[++SP] = pc;
	pc = nnn;
}

void
Chip8::OP_3xkk() {
	if (V[x] == kk) pc += 2;
}

void
Chip8::OP_4xkk() {
	if (V[x] != kk) pc += 2;
}

void
Chip8::OP_5xy0() {
	if (V[x] == V[y]) pc += 2;
}

void
Chip8::OP_6xkk() {
	V[x] = kk;
}

void
Chip8::OP_7xkk() {
	V[x] += kk;
}

void
Chip8::OP_8xy0() {
	V[x] = V[y];
}

void
Chip8::OP_8xy1() {
	V[x] |= V[y];
}

void
Chip8::OP_8xy2() {
	V[x] &= V[y];
}

void
Chip8::OP_8xy3() {
	V[x] ^= V[y];
}

void
Chip8::OP_8xy4() {
	const auto res = V[x] + V[y];

	V[x]   = res;
	V[0xF] = res >> 8;
}

void
Chip8::OP_8xy5() {
	const auto res = V[x] - V[y];

	V[x]   = res;
	V[0xF] = !(res >> 8);
}

void
Chip8::OP_8xy6() {
	V[0xF] = V[x] & 0x1;
	V[x] >>= 1;
}

void
Chip8::OP_8xy7() {
	const auto res = V[y] - V[x];

	V[x]   = res;
	V[0xF] = !(res >> 8);
}

void
Chip8::OP_8xyE() {
	V[0xF] = V[x] >> 7;
	V[x] <<= 1;
}

void
Chip8::OP_9xy0() {
	if (V[x] != V[y]) pc += 2;
}

void
Chip8::OP_Annn() {
	I = nnn;
}

void
Chip8::OP_Bnnn() {
	pc = nnn + V[0x0];
}

void
Chip8::OP_Cxkk() {
	const auto random = std::uniform_int_distribution<>(0,255)(rnd);

	V[x] = random & kk;
}

void
Chip8::OP_Dxyn() {
	unsigned short pixel;

	unsigned char xPos = V[x] % W;
	unsigned char yPos = V[y] % H;

	V[0xF] = 0;
	for (unsigned int row = 0; row < n; ++row) {
		pixel = Mem[I + row];
		
		for (unsigned int col = 0; col < 8; ++col) {
			unsigned char spritePixel = pixel & (0x80 >> col);

			if (spritePixel) {
				unsigned int &disp = Display[(yPos + row) * W + (xPos + col)];

				if (disp == 0xFFFFFFFF)
					V[0xF] = 1;

				disp ^= 0xFFFFFFFF;
			}
		}
	}
}

void
Chip8::OP_Ex9E() {
	if (Keys[V[x]]) pc += 2;
}

void
Chip8::OP_ExA1() {
	if (!(Keys[V[x]])) pc += 2;
}

void
Chip8::OP_Fx07() {
	V[x] = DelayTimer;
}

void
Chip8::OP_Fx0A() {
	for (int i = 0; i < 16; ++i) {
		if (Keys[i]) V[x] = i;
		else pc -= 2;
	}
}

void
Chip8::OP_Fx15() {
	DelayTimer = V[x];
}

void
Chip8::OP_Fx18() {
	SoundTimer = V[x];
}

void
Chip8::OP_Fx1E() {
	I += V[x];
}

void
Chip8::OP_Fx29() {
	I = (5 * V[x]);
}

void
Chip8::OP_Fx33() {
	Mem[I+0] = (V[x]/100) % 10;
	Mem[I+1] = (V[x]/10) % 10;
	Mem[I+2] = (V[x]/1) % 10;
}

void
Chip8::OP_Fx55() {
	for (unsigned int i = 0; i <= x; ++i)
		Mem[I + i] = V[i];
}

void
Chip8::OP_Fx65() {
	for (unsigned int i = 0; i <= x; ++i)
		V[i] = Mem[I + i];
}
