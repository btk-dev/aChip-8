#include "c8.h"

#include <SDL.h>
#include <iostream>
#include <thread>
#include <chrono>

#undef main

uint8_t keymap[16] = {
	SDLK_x,
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_q,
	SDLK_w,
	SDLK_e,
	SDLK_a,
	SDLK_s,
	SDLK_d,
	SDLK_z,
	SDLK_c,
	SDLK_4,
	SDLK_r,
	SDLK_f,
	SDLK_v
};

const int height = 1024;
const int width = 512;

int main() {

	c8 c;
	c.init();

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow("CHIP-8 Interpreter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_RenderSetLogicalSize(renderer, width, height);

	SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

	uint32_t pixels[2048];

	while (true) {
		c.clockCycle();

		if (c.drawFlag) {
			c.drawFlag = false;

			for (int i = 0; i < 2048; i++) {
				uint8_t p = c.gfx[i];
				pixels[i] = (0x00FFFFFF * p) | 0xFF000000;
			}

			SDL_UpdateTexture(tex, NULL, pixels, 64 * sizeof(Uint32));
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, tex, NULL, NULL);
			SDL_RenderPresent(renderer);
		}

		std::this_thread::sleep_for(std::chrono::microseconds(1500));
	}

	return 0;
}