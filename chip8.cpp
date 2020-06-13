#include <fstream>

static constexpr const unsigned int W = 64, H = 32;

struct chip8 {
	union {
		// Memory of the chip 8, 4096 bytes
		unsigned char Mem[0x1000] {0};
		
		struct {
			unsigned char Reg[16], DelayTimer, SoundTimer, Keys[16], SP;
			unsigned int DisplayMem[W * H];
			unsigned short Stack[16], PC, RegIdx;
		};
	};

	void loadROM(const char *filename) {
		std::ifstream(filename, std::ios::binary | std::ios::ate);
	}
};




