#define _CRT_SECURE_NO_WARNINGS
#include "c8.h"

c8::c8() {}

c8::~c8() {}

void c8::init() {
	//first memory address used by programs
	c8::pc = 0x200;
	c8::opcode = 0;
	c8::I = 0;
	c8::sp = 0;
	//clear keys, stack and registers
	for (int i = 0; i < 16; i++) {
		c8::stack[i] = 0;
		c8::V[i] = 0;
		c8::key[i] = 0;
	}
	//clear memory
	for (int i = 0; i < 4096; i++) {
		c8::mem[i] = 0;
	}
	//clear graphics
	for (int i = 0; i < 2048; i++) {
		c8::gfx[i] = 0;
	}
	//load font set
	for (int i = 0; i < 80; i++) {
		c8::mem[i] = c8::font_set[i];
	}

	c8::delay_timer = 0;
	c8::sound_timer = 0;
}

bool c8::load(const char* file_name) {
	c8::init();

	FILE* rom = fopen(file_name, "rb");
	if (rom == NULL)
		return false;

	fseek(rom, 0, SEEK_END);
	long rom_size = ftell(rom);
	rewind(rom);

	char* buffer = (char*)malloc(sizeof(char) * rom_size);
	if (buffer == NULL)
		return false;

	size_t result = fread(buffer, sizeof(char), (size_t)rom_size, rom);
	if (result != rom_size)
		return false;

	if ((4096 - 512) > rom_size) {
		for (int i = 0; i < rom_size; i++)
			c8::mem[i + 512] = (uint8_t)buffer[i];
	}
	else
		return false;

	fclose(rom);
	free(buffer);

	return true;
}

void c8::clockCycle() {
	//fetch opcode
	//opcodes are 2 bytes, so retrieve the first byte and shift left 2 bytes to bitwise or with second byte retrieved
	c8::opcode = c8::mem[c8::pc] << 8 | c8::mem[c8::pc + 1];

	//decode opcode
	//switch statement for now. Possible pointer function array later
	//AND mask first nibble to retrieve opcode category
	switch (c8::opcode & 0xF000) {
		//bit shifting 8\4 due to AND masking creating trailing 0s
	case 0x0000:
		//2 possibilities, switch on last nibble
		switch (c8::opcode & 0x000F) {
		case 0x0000:
			//clear display
			for (int i = 0; i < 2048; i++)
				c8::gfx[i] = 0;
			c8::drawFlag = true;
			c8::pc += 2;
			break;
		case 0x000E:
			//return from subroutine
			c8::sp--;
			c8::pc = c8::stack[c8::sp];
			c8::pc += 2;
			break;
		default:
			printf("Unrecognized opcode");
			break;
		}
	case 0x1000:
		//opcode 1nnn. Jump to location nnn
		c8::pc = (c8::opcode & 0x0FFF);
		break;
	case 0x2000:
		//call subroutine at 2nnn
		c8::stack[c8::sp] = c8::pc;
		c8::sp++;
		c8::pc = (c8::opcode & 0x0FFF);
		break;
	case 0x3000:
		//opcode 3xkk. Skip next instruction if V[x] = kk
		if (c8::V[(c8::opcode & 0x0F00) >> 8] == (c8::opcode & 0x00FF))
			c8::pc += 4;
		else
			c8::pc += 2;
		break;
	case 0x4000:
		//opcode 4xkk. Skip next instruction if V[x] != kk
		if (c8::V[(c8::opcode & 0x0F00) >> 8] != (c8::opcode & 0x00FF))
			c8::pc += 4;
		else
			c8::pc += 2;
		break;
	case 0x5000:
		//opcode 5xy0. Skip next instruction if V[x] == V[y]
		if (c8::V[(c8::opcode & 0x0F00) >> 8] == c8::V[c8::opcode & 0x00F0] >> 4)
			c8::pc += 4;
		else
			c8::pc += 2;
		break;
	case 0x6000:
		//opcode 6xkk. Put value kk in register V[x]
		c8::V[(c8::opcode & 0x0F00) >> 8] = (c8::opcode & 0x00FF);
		c8::pc += 2;
		break;
	case 0x7000:
		//opcode 7xkk. add immediate
		c8::V[(c8::opcode & 0x0F00) >> 8] = c8::V[(c8::opcode & 0x0F00) >> 8] + (c8::opcode & 0x00FF);
		c8::pc += 2;
		break;
	case 0x8000:
		//arithmetic section
		//Switch on last nibble of opcode
		switch (c8::opcode & 0x000F) {
		case 0x0000:
			//set V[x} = V[y]
			c8::V[(c8::opcode & 0x0F00) >> 8] = c8::V[(c8::opcode & 0x00F0) >> 4];
			c8::pc += 2;
			break;
		case 0x0001:
			//bitwise or on values V[x] and V[y]. Store in V[x]
			c8::V[(c8::opcode & 0x0F00) >> 8] = c8::V[(c8::opcode & 0x0F00) >> 8] | c8::V[(c8::opcode & 0x00F0) >> 4];
			c8::pc += 2;
			break;
		case 0x0002:
			//bitwise and on values V[x] and V[y]. Store in V[x]
			c8::V[(c8::opcode & 0x0F00) >> 8] = c8::V[(c8::opcode & 0x0F00) >> 8] & c8::V[(c8::opcode & 0x00F0) >> 4];
			c8::pc += 2;
			break;
		case 0x0003:
			//Bitwise xor on values V[x] and V[y]. Store in V[x]
			c8::V[(c8::opcode & 0x0F00) >> 8] = c8::V[(c8::opcode & 0x0F00) >> 8] ^ c8::V[(c8::opcode & 0x00F0) >> 4];
			c8::pc += 2;
			break;
		case 0x0004:
			//Set V[x] = V[x] + V[y] with V[F] as the carry
			c8::V[(c8::opcode & 0x0F00) >> 8] += c8::V[(c8::opcode & 0x00F0) >> 4];
			if (c8::V[(c8::opcode & 0x00F0) >> 4] > (0xFF - c8::V[(c8::opcode & 0x0F00) >> 8]))
				c8::V[0xF] = 1;
			else
				c8::V[0xF] = 0;
			c8::pc += 2;
			break;
		case 0x0005:
			//Set V[x] = V[y] - V[x]. If V[x] > V[y] set V[F] = 0, else 1
			c8::V[(c8::opcode & 0x0F00) >> 8] = c8::V[(c8::opcode & 0x00F0) >> 4] - c8::V[(c8::opcode & 0x0F00) >> 8];
			if (c8::V[(c8::opcode & 0x00F0) >> 4] > c8::V[(c8::opcode & 0x0F00) >> 8])
				c8::V[0xF] = 0;
			else
				c8::V[0xF] = 1;
			c8::pc += 2;
			break;
		case 0x0006:
			//Set V[x] = V[x] shift right 1. If the least significant bit of V[x] = 1 then V[F] = 1, else 0
			c8::V[0xF] = c8::V[(c8::opcode & 0x0F00) >> 8] & 0x1;
			c8::V[(c8::opcode & 0x0F00) >> 8] >>= 1;
			c8::pc += 2;
			break;
		case 0x0007:
			//Set V[x] = V[y] - V[x]. If V[y] >  V[x], V[F] = 1
			c8::V[(c8::opcode & 0x0F00) >> 8] = c8::V[(c8::opcode & 0x00F0) >> 4] - c8::V[(c8::opcode & 0x0F00) >> 8];
			if (c8::V[(c8::opcode & 0x0F00) >> 8] > c8::V[(c8::opcode & 0x00F0) >> 4])
				c8::V[0xF] = 0;
			else
				c8::V[0xF] = 1;
			c8::pc += 2;
			break;
		case 0x000E:
			//If the most significant bit of V[x] is 1 then V[F] is set to 1.
			c8::V[0xF] = c8::V[(c8::opcode & 0x0F00) >> 8] >> 7;
			c8::V[(c8::opcode & 0x0F00) >> 8] <<= 1;
			c8::pc += 2;
			break;
		default:
			printf("Unrecognized opcode");
			break;
		}
	case 0x9000:
		//opcode 9xy0. Skip next instruction if V[x] != V[y]
		if (c8::V[(c8::opcode & 0x0F00) >> 8] != c8::V[c8::opcode & 0x00F0] >> 4)
			c8::pc += 4;
		else
			c8::pc += 2;
		break;
	case 0xA000:
		//opcode Annn. Set index register to nnn
		c8::I = (c8::opcode & 0x0FFF);
		c8::pc += 2;
		break;
	case 0xB000:
		//opcode Bnnn. Jump to nnn plus V[0]
		c8::pc = c8::V[0] + (c8::opcode & 0x0FFF);
		break;
	case 0xC000:
		//opcode Cxnn. set V[x] = random number (0-255) AND nn
		c8::V[(c8::opcode & 0x0F00) >> 8] = (c8::opcode & 0x00FF) & std::uniform_int_distribution<>(0, 255)(c8::rnd);
		c8::pc += 2;
		break;
	case 0xD000:
		//drawing opcode
		unsigned short x = c8::V[(c8::opcode & 0x0F00) >> 8];
		unsigned short y = c8::V[(c8::opcode & 0x00F0) >> 4];
		unsigned short h = c8::opcode & 0x000F;
		unsigned short p;
		c8::V[0xF] = 0;
		for (int i = 0; i < h; i++) {
			p = c8::mem[c8::I + i];
			for (int j = 0; j < 8; j++) {
				if ((p & (0x80 >> j)) != 0) {
					if (c8::gfx[(x + j + ((y + i) * 64))] == 1)
						c8::V[0xF] = 1;
					c8::gfx[x + j + ((y + i) * 64)] ^= 1;
				}
			}
		}
		c8::drawFlag = true;
		c8::pc += 2;
		break;
	case 0xE000:
		//2 key related instructions. Switch on AND mask last byte
		switch (c8::opcode & 0x00FF) {
		case 0x009E:
			//opcode Ex9E. Skip next instruction if key with value V[x] is pressed
			if (c8::key[c8::V[(c8::opcode & 0x0F00) >> 8]] == 0)
				c8::pc += 2;
			else
				c8::pc += 2;
			break;
		case 0x00A1:
			//opcode ExA1. Skip next instruction if key with value V[x] is not pressed
			if (c8::key[c8::V[(c8::opcode & 0x0F00) >> 8]] != 0)
				c8::pc += 2;
			else
				c8::pc += 2;
			break;
		default:
			printf("Unrecognized opcode");
			break;
		}
	case 0xF000:
		//misc opcodes. Switch on AND mask last byte
		switch (c8::opcode & 0x00FF) {
		case 0x0007:
			//register V[x] is set equal to delay timer
			c8::V[(c8::opcode & 0x0F00) >> 8] = c8::delay_timer;
			c8::pc += 2;
			break;
		case 0x000A:
			//wait for key press, store value of the key in V[x]
		{
			bool key_pressed = false;

			for (int i = 0; i < 16; i++) {
				if (c8::key[i] != 0) {
					c8::V[(c8::opcode & 0x0F00) >> 8] = i;
					key_pressed = true;
				}
			}

			if (!key_pressed)
				return;

			c8::pc != 2;
		}
		break;
		case 0x0015:
			//delay timer set equal to value in V[x]
			c8::delay_timer = c8::V[(c8::opcode & 0x0F00) >> 8];
			c8::pc += 2;
			break;
		case 0x0018:
			//sound timer set equal to value in V[x]
			c8::sound_timer = c8::V[(c8::opcode & 0x0F00) >> 8];
			c8::pc += 2;
			break;
		case 0x001E:
			//add I to value in V[x] and store in I
			if (c8::I + c8::V[(c8::opcode & 0x0F00) >> 8] > 0xFFF)
				c8::V[0xF] = 1;
			else
				c8::V[0xF] = 0;
			c8::I += c8::V[(c8::opcode & 0x0F00) >> 8];
			c8::pc += 2;
			break;
		case 0x0029:
			//set I to location of sprite for digit V[x]
			c8::I = c8::V[(c8::opcode & 0x0F00) >> 8] * 0x5;
			c8::pc += 2;
			break;
		case 0x0033:
			//store BCD rep of V[x] in memory locations I, I+1, I+2
			c8::mem[c8::I] = c8::V[(c8::opcode & 0x0F00) >> 8] / 100;
			c8::mem[c8::I + 1] = (c8::V[(c8::opcode & 0x0F00) >> 8] / 10) % 10;
			c8::mem[c8::I + 2] = c8::V[(c8::opcode & 0x0F00) >> 8] % 10;
			c8::pc += 2;
			break;
		case 0x0055:
			//store registers V[0] through V[x] in memory starting at I
			for (int i = 0; i < ((c8::opcode & 0x0F00) >> 8); i++) {
				c8::mem[c8::I + i] = c8::V[i];
			}
			c8::pc += 2;
			break;
		case 0x0065:
			//read registers V[0] through V[x] into memory starting at I
			for (int i = 0; i < ((c8::opcode & 0x0F00) >> 8); i++) {
				c8::V[i] = c8::mem[c8::I + i];
			}
			c8::pc += 2;
			break;
		default:
			printf("Unrecognized opcode");
			break;
		}
	default:
		printf("Unrecognized opcode");
		break;
	}

	//update timers
	if (c8::delay_timer > 0)
		c8::delay_timer--;
	if (c8::sound_timer > 0) {
		if (c8::sound_timer == 1) {
			//play sound
		}
		c8::sound_timer--;
	}
}