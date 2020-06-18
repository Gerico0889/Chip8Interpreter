#include <chrono>
#include <unordered_map>
#include <SDL2/SDL.h>

#include "chip8.hpp"

int main(int argc, char **argv) {

	int videoScale = std::stoi(argv[1]);
	int cycleDelay = std::stoi(argv[2]);

	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = SDL_CreateWindow(argv[3], 0, 0, W * videoScale, H * videoScale, SDL_WINDOW_SHOWN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, W, H);

	Chip8 cpu;
	cpu.loadRom(argv[3]);
	int pitch = 4 * W;

	auto lastCycleTime = std::chrono::high_resolution_clock::now();
	bool quit = false;

	std::unordered_map<int,int> keymap{
        {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
        {SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0xD},
        {SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xE},
        {SDLK_z, 0xA}, {SDLK_x, 0x0}, {SDLK_c, 0xB}, {SDLK_v, 0xF},
        {SDLK_5, 0x5}, {SDLK_6, 0x6}, {SDLK_7, 0x7},
        {SDLK_8, 0x8}, {SDLK_9, 0x9}, {SDLK_0, 0x0}, {SDLK_ESCAPE,-1}
    };

	while (!quit) {
		for (SDL_Event ev; SDL_PollEvent(&ev); )
			switch (ev.type)
			{
				case SDL_QUIT: quit = true; break;
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					auto key = keymap.find(ev.key.keysym.sym);
					if (key == keymap.end()) break;
					if (key->second == -1) { quit = true; break; }
					cpu.Keys[key->second] = ev.type == SDL_KEYDOWN;
			}

		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

		if (dt > cycleDelay) {
			lastCycleTime = currentTime;

			cpu.cycle();
			
			SDL_UpdateTexture(texture, nullptr, cpu.Display, pitch);
			SDL_RenderCopy(renderer, texture, nullptr, nullptr);
			SDL_RenderPresent(renderer);
		}
	
	}

	SDL_Quit();
	return 0;
}
