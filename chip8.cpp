#include <fstream>

static constexpr const unsigned int W = 64, H = 32;

struct chip8 {
	union {
		// Memory of the chip 8, 4096 bytes
		unsigned char Mem[0x1000] {0};
		
		struct {
			unsigned char Reg[16], DelayTimer, SoundTimer, Keys[16], SP;
			unsigned int DisplayMem[W * H], Fonts[80];
			unsigned short Stack[16], PC, RegIdx;
		};
	};

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
				// starting from the end of each font letter/number
				*begin++ = (f >> shift & 0xF);
			}

		}
	}

	void loadROM(const char *filename, unsigned pos = 0x200) {
		for (std::ifstream rom(filename, std::ios::binary); rom.good() ;)
			Mem[pos++ & 0xFFF] = rom.get();
	}

};




