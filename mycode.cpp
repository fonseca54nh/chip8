#include <iostream>
#include <fstream>
#include <SDL.h>
#include <time.h>
#include "stdint.h"
#include <cstdio>
#include <cstdlib>
#include <random>
#include <chrono>
#include <thread>

using namespace std;

class chip8 {
public:
	chip8();
	~chip8();
	
	bool drawFlag;
	void EmulateCycle();
	bool load (const char * filename);

	// Chip8
	uint8_t  gfx[2048];
	unsigned char  key[16];

	unsigned short  pc;
	unsigned short 	opcode;
	unsigned short 	I;
	unsigned short	sp;			

	unsigned char  v[16];			
	unsigned short stack[16];		
	unsigned char  memory[4096];			

	unsigned char  delay_timer;		
	unsigned char  sound_timer;		

	
	void Init();
	
};

unsigned char chip8_fontset[80]
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

chip8::chip8() {}
chip8::~chip8() {}

void chip8::Init()
{
	pc = 0x200;
	I = 0;
	sp = 0;
	opcode = 0;

	//clear the display 
	for (int i = 0; i < 2048; ++i)
		gfx[i] = 0;

	//clear the stack
	for (int i = 0; i < 16; ++i)
		stack[i] = 0;

	//clear the memory
	for (int i = 0; i < 4096; ++i)
		memory[i] = 0;

	//clear the keys
	for (int i = 0; i < 16; ++i)
		key[i] = v[i] = 0;

	//load the fontset into memory
	for (int i = 0; i < 80; ++i)
		memory[i] = chip8_fontset[i];

	delay_timer = 0;
	sound_timer = 0;

	drawFlag = true;


	srand(time(NULL));

}

void chip8::EmulateCycle()
{
	opcode = memory[pc] << 8 | memory[pc + 1];

	switch (opcode & 0xF000)
	{
	case 0x0000:
		switch (opcode & 0x000F)
		{
		case 0x0000:

			for (int i = 0; i < 2048; ++i)
				gfx[i] = 0;
			drawFlag = true;
			pc += 2;
			break;

		case 0x000E:
			--sp;
			pc = stack[sp];
			pc += 2;
			break;

		default:
			cout << "Invalid opcode" << opcode << endl;
		}
		break;

	case 0x1000:
		pc = opcode & 0x0FFF;
		break;

	case 0x2000:
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;

	case 0x3000:
		if (v[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x4000:
		if (v[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x5000:
		if (v[(opcode & 0x0F00) >> 8] == v[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;

	case 0x6000:
		v[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
		break;

	case  0x7000:
		v[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
		break;

	case 0x8000:
		switch (opcode & 0x000F)
		{
		case 0x0000:
			v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0001:
			v[(opcode & 0x0F00) >> 8] |= v[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0002:
			v[(opcode & 0x0F00) >> 8] &= v[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0003:
			v[(opcode & 0x0F00) >> 8] ^= v[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0004:
			if (v[(opcode & 0x00F0) >> 4] > (0xFF - v[(opcode & 0x0F00) >> 8]))
				v[0xF] = 1;
			else
				v[0xF] = 0;
			v[(opcode & 0x0F00) >> 8] += v[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0005:
			if (v[(opcode & 0x00F0) >> 4] > v[(opcode & 0x0F00) >> 8])
				v[0xF] = 0;
			else
				v[0xF] = 1;
			v[(opcode & 0x0F00) >> 8] -= v[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0006:
			v[0xF] = v[(opcode & 0x0F00) >> 8] & 0x1;
			v[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

		case 0x0007:
			if (v[(opcode & 0x0F00) >> 8] > v[(opcode & 0x00F0) >> 4])
				v[0xF] = 0;
			else
				v[0xF] = 1;
			v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x00F0) >> 4] - v[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x000E:
			v[0xF] = v[(opcode & 0x0F00) >> 8] >> 7;
			v[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			break;

		default:
			std::cerr << "Invalid opcode [0x8000]: 0x" << opcode << endl;
		}
		break;

	case 0x9000:
		if (v[(opcode & 0x0F00) >> 8] != v[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;

	case 0xA000:
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	case 0xB000:
		pc = (opcode & 0x0FFF) + v[0];
		break;

	case 0xC000:
		v[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
		pc += 2;
		break;

	case 0xD000:
	{
		unsigned short x = v[(opcode & 0x0F00) >> 8];
		unsigned short y = v[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		v[0xF] = 0;
		for (int yline = 0; yline < height; yline++)
		{
			pixel = memory[I + yline];

			for (int xline = 0; xline < 8; xline++)
			{
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)
					{
						v[0xF] = 1;
					}
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}
		drawFlag = true;
		pc += 2;
	}
	break;

	case 0xE000:
		switch (opcode & 0x00FF)
		{
		case 0x009E:
			if (key[v[(opcode & 0x0F00) >> 8]] != 0)
				pc += 4;
			else
				pc += 2;
			break;

		case 0x00A1:
			if (key[v[(opcode & 0x0F00) >> 8]] == 0)
				pc += 4;
			else
				pc += 2;

			break;

		default:
			cout << "Invalid opcode" << opcode << endl;
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF)
		{
		case 0x0007:
			v[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x000A:
		{
			bool keypress = false;
			for (int i = 0; i < 16; ++i)
			{
				if (key[i] != 0)
					v[(opcode & 0x0F00) >> 8] = i;
				keypress = true;
			}

			if (!keypress)
				return;

			pc += 2;
		}
		break;

		case 0x0015:
			v[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x0018:
			v[(opcode & 0x0F00) >> 8] = sound_timer;
			pc += 2;
			break;

		case 0x001E:
			if (I + v[(opcode & 0x0F00) >> 8] > 0xFFF)
				v[0xF] = 1;
			else
				v[0xF] = 0;
			I += v[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0029:
			I = v[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			break;

		case 0x0033:
			memory[I] = v[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (v[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (v[(opcode & 0x0F00) >> 8] / 100) % 10;
			pc += 2;
			break;

		case 0x0055:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				memory[I + i] = v[i];
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		case 0x0065:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				v[i] = memory[I + i];
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		default:
			std::cout << "Invalid opcode" << opcode << endl;
		}
		break;

	default:
		std::cout << "Invalid opcode " << opcode << std::endl;
	}

	if (delay_timer > 0)
		--delay_timer;

	if (sound_timer > 0)
	{
		if (sound_timer == 1)
			std::cout << "BEEP" << std::endl;
		--sound_timer;
	}
}
	
bool chip8::load(const char * filename)
{
	Init();
	printf("Loading: %s\n", filename);

	// Open file
	FILE * pFile = fopen(filename, "rb");
	if (pFile == NULL)
	{
		fputs("File error", stderr);
		return false;
	}

	// Check file size
	fseek(pFile, 0, SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);

	// Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL)
	{
		fputs("Memory error", stderr);
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread(buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		fputs("Reading error", stderr);
		return false;
	}

	// Copy buffer to Chip8 memory
	if ((4096 - 512) > lSize)
	{
		for (int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");

	// Close file, free buffer
	fclose(pFile);
	free(buffer);

	return true;
}

int main(int argc, char** argv)
{
	chip8 cpu;
	cpu.load(argv[1]);

	SDL_Window* window = NULL;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		std::cout << "Merda" << std::endl;
	}
	else {
		const int WIDTH = 64;
		const int HEIGHT = 32;


		window = SDL_CreateWindow(argv[1], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
		SDL_Renderer* ren = SDL_CreateRenderer(window, -1, 0);
		SDL_RenderSetLogicalSize(ren, WIDTH, HEIGHT);
		SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

		// game loop
		uint32_t pixels[2048];
		bool running = false;

		while (!running)
		{
			cpu.EmulateCycle();

			SDL_Event e;

			while (SDL_PollEvent(&e) != 0)
			{
				if (e.type == SDL_QUIT)
					running = true;

				if (cpu.drawFlag)
				{
					cpu.drawFlag = false;
					for (int i = 0; i < 2048; ++i)
					{
						uint8_t pixel = cpu.gfx[i];
						pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
					}
				}

				SDL_UpdateTexture(tex, NULL, pixels, 64 * sizeof(Uint32));
				SDL_RenderClear(ren);
				SDL_RenderCopy(ren, tex, nullptr, nullptr);
				SDL_RenderPresent(ren);
			}
			SDL_Delay(9);
			//std::this_thread::sleep_for(std::chrono::microseconds(1000/60));

		}
	}
	SDL_Quit();
	return EXIT_SUCCESS;
}