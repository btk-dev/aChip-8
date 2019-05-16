#pragma once
#include <random>

class c8 {
public:
	c8();
	~c8();
	
	//screen size of the CHIP8 is 64 pixels by 32 pixels
	//drawing is done using xor. 1 = on 0 = off.
	unsigned short gfx[64 * 32];

	//16 key keyboard for the CHIP8
	unsigned char key[16];

	//initialize all variables to beginning state
	void init();

	//simulate one clock cycle of the cpu
	void clockCycle();

	//load file
	bool load(const char* file_path);

	//indicate if the screen needs to be redrawn
	bool drawFlag = false;
private:
	//35 possible opcodes
	unsigned short opcode;
	//CHIP8 has 0x1000 memory with the first 0x200 being reserved for interpreter
	unsigned char mem[0x1000];
	//15 8-bit general registers and 1 register for special use
	unsigned char V[16];
	//Index register and program counter from 0x000 - 0xFFF
	unsigned short I;
	unsigned short pc;
	//CHIP8 has no interupts but 2 timers
	unsigned char delay_timer;
	unsigned short sound_timer;
	//CHIP8 has a 16 item stack (12 in most newer implementations) due to jump instruction, and a stack pointer
	unsigned short stack[16];
	unsigned short sp;

	//CHIP8 fontset. Loaded into first 80 memory sectores
	unsigned char font_set[80] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	//for use with random number instruction
	std::mt19937 rnd{};
};