#include <fstream>
#include <random>

static constexpr const unsigned int W = 64, H = 32;

struct chip8 {
	union {
		// Memory of the chip 8, 4096 bytes
		unsigned char Mem[0x1000] {0};
		
		struct {
			unsigned char Reg[16], DelayTimer, SoundTimer, Keys[16], SP;
			unsigned int DisplayMem[W * H], Fonts[80];
			unsigned short Stack[16], PC, RegIdx, opcode;
		};
	};

	std::mt19937 rnd{};

	chip8() {

		// Load fonts
		auto *begin = Fonts;
		constexpr const unsigned fList[] = {0xF999F, 0x26227, 0xF1F87, 0xF1F1F,
                                            0x99F11, 0xF8F1F, 0xF8F9F, 0xF1244,
                                            0xF9F9F, 0xF9F1F, 0xF9F99, 0xE9E9E,
                                            0xF888F, 0xE999E, 0xF8F8F, 0xF8F88};

		for (unsigned f : fList) {
			for (int shift = 16; shift >= 0; shift -= 4) {
				// Shift the bits and do a bitwise and with 0xF
				// in order to take 4 bits at a time
				// starting from the beginning of each font letter/number
				*begin++ = (f >> shift & 0xF);
			}

		}
	}

	void loadROM(const char *filename, unsigned pos = 0x200) {
		for (std::ifstream rom(filename, std::ios::binary); rom.good() ;)
			Mem[pos++ & 0xFFF] = rom.get();
	}

	// Instructions
	
	// CLS: Clear the display
	void OP_00E0() {
		for (auto &pixel : DisplayMem)
			pixel = 0;
	}

	// RET: Return from a subroutine
	void OP_00EE() {
		Stack[SP-- % 16];
	}

	// JP addr: jump to address nnn
	void OP_1nnn() {
		PC = opcode & 0xFFFu;
	}

	//CALL: call a subroutine
	void OP_2nnn() {
		Stack[SP++] = PC;
		PC = opcode & 0xFFFu;
	}

	// SE Vx, byte: if Vx == kk increase the PC by 2
	void OP_3xkk() {
		// Vx indicates the 4 least significant bits of the high byte
		const unsigned Vx = (opcode & 0x0F00) >> 8u;

		// kk indicates the lowest 8bits of the instruction
		const unsigned kk = opcode & 0x00FF;

		if (Reg[Vx] == kk) PC += 2;
	}

	// SNE Vx, byte: if Vx != kk increase the PC by 2
	void OP_4xkk() {
		// Vx indicates the 4 least significant bits of the high byte
		const unsigned Vx = (opcode & 0x0F00) >> 8u;

		// kk indicates the lowest 8bits of the instruction
		const unsigned kk = opcode & 0x00FF;

		if (Reg[Vx] != kk) PC += 2;
	}

	// SE Vx, Vy: if Vx == Vy increase the PC by 2
	void OP_5xy0() {
		const unsigned Vx = (opcode & 0x0F00) >> 8u;

		// Vy indicates the 4 most significant bits of the low byte
		const unsigned Vy = (opcode & 0x00F0) >> 4u;

		if (Reg[Vx] == Reg[Vy]) PC += 2;
	}

	// LD Vx, byte: set Vx = kk
	void OP_6xkk() {
		const unsigned Vx = (opcode & 0x0F00) >> 8u;
		const unsigned kk = opcode & 0x00FF;

		Reg[Vx] = kk;
	}

	// ADD Vx, byte: set Vx = Vx + kk
	void OP_7xkk() {
		const unsigned Vx = (opcode & 0x0F00) >> 8u;
		const unsigned kk = opcode & 0x00FF;

		Reg[Vx] += kk;
	}

	// LD Vx, Vy: set Vx = Vy
	void OP_8xy0() {
		const unsigned Vx = (opcode & 0x0F00) >> 8u;
		const unsigned Vy = (opcode & 0x00F0) >> 4u;

		Reg[Vx] = Reg[Vy];
	}

	// OR Vx, Vy: set Vx = Vx or Vy
	void OP_8xy1() {
		const unsigned Vx = (opcode & 0x0F00) >> 8u;
		const unsigned Vy = (opcode & 0x00F0) >> 4u;

		Reg[Vx] |= Reg[Vy];
	}

	// AND Vx, Vy: set Vx = Vx and Vy
	void OP_8xy2() {
		const unsigned Vx = (opcode & 0x0F00) >> 8u;
		const unsigned Vy = (opcode & 0x00F0) >> 4u;

		Reg[Vx] &= Reg[Vy];
	}

	// XOR Vx, Vy: set Vx = Vx xor Vy
	void OP_8xy3() {
		const unsigned Vx = (opcode & 0x0F00) >> 8u;
		const unsigned Vy = (opcode & 0x00F0) >> 4u;

		Reg[Vx] ^= Reg[Vy];
	}

	// ADD Vx, Vy: set Vx = Vx + Vy, set VF = carry
	void OP_8xy4() {
		const unsigned Vx = (opcode & 0x0F00) >> 8u;
		const unsigned Vy = (opcode & 0x00F0) >> 4u;

		unsigned res = Reg[Vx] + Reg[Vy];
		Reg[0xF] = res >> 8u;
		Reg[Vx] = res & 0x00FFu;
	}

	// SUB Vx, Vy: set Vx = Vx - Vy, set VF = NOT borrow
	void OP_8xy5() {
		const unsigned Vx = (opcode & 0x0F00) >> 8u;
		const unsigned Vy = (opcode & 0x00F0) >> 4u;

		unsigned res = Reg[Vx] - Reg[Vy];
		Reg[0xF] = !(res >> 8u);
		Reg[Vx] = res;
	}

	// SHR Vx {, Vy}: set Vx = Vx SHR 1
	void OP_8xy6() {
		const unsigned Vx = (opcode & 0x0F00) >> 8u;

		Reg[0xF] = Reg[Vx] & 0x1u;
		Reg[Vx] >>= 1;
	}

	// SUBN Vx, Vy: set Vx = Vy - Vx, VF = NOT borrow
	void OP_8xy7() {
		const unsigned Vx = (opcode & 0x0F00) >> 8u;
		const unsigned Vy = (opcode & 0x00F0) >> 4u;

		unsigned res = Reg[Vy] - Reg[Vx];
		Reg[0xF] = !(res >> 8u);
		Reg[Vx] = res;
	}

	// SHL Vx {, Vy}: set Vx = Vx SHL 1
	void OP_8xyE() {
		const unsigned Vx = (opcode & 0x0F00) >> 8u;

		Reg[0xF] = (Reg[Vx] & 0x80u) >> 7u;
		Reg[Vx] <<= 1;
	}

};




