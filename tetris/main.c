#include <stdio.h>
#include <string.h>
#include <time.h>
#include <SDL/SDL.h>

enum {
	CELL_SIZE = 20,
	GRID_WIDTH = 10,
	GRID_HEIGHT = 20,
};


const char STONE_DATA[][16] = {
	{0x0,0xa,0x0,0x0,0x5,0xf,0x5,0x5,0x0,0xa,0x0,0x0,0x0,0xa}, // I
	{0x0,0x0,0x0,0x0,0x0,0xf,0xf,0x0,0x0,0xf,0xf}, // O
	{0x4,0x5,0x8,0x0,0xa,0xf,0xa,0x0,0x2,0x5,0x1}, // L
	{0x2,0x5,0x4,0x0,0xa,0xf,0xa,0x0,0x1,0x5,0x8}, // J
	{0x0,0xe,0x0,0x0,0x7,0xf,0xd,0x0,0x0,0xb,0x0}, // T
	{0x0,0x0,0xa,0x0,0x5,0xf,0xa,0x0,0x0,0xf,0x5}, // Z
	{0xa,0x0,0x0,0x0,0xa,0xf,0x5,0x0,0x5,0xf,0x0}, // S
	{0x9,0x5,0x3,0x0,0xa,0xf,0xa,0x0,0xc,0x5,0x6}, // big T
};



SDL_Surface* screen;

void draw_cell(int x, int y, uint32_t c) {
	SDL_Rect rect = {
		x * CELL_SIZE, y * CELL_SIZE,
		CELL_SIZE, CELL_SIZE
	};
	SDL_FillRect(screen, &rect, c);
}


int pos_x;
int pos_y;
int rotation;
int stone;
int lines;
int drop;
int falling;
int cells[GRID_HEIGHT][GRID_WIDTH];


int collision(int* over) {
	if (over) *over = 0;
	int i;
	for (i = 0; i < 16; i++) {
		if (STONE_DATA[stone][i] >> rotation & 1) {
			int x = pos_x + i % 4;
			int y = pos_y + i / 4;
			if (x < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT) return 1;
			if (y >= 0 && cells[y][x]) return 1;
			else if (over && cells[y][x]) *over = 1;
		}
	}
	return 0;
}

void new_stone(void) {
	pos_x = GRID_WIDTH / 2 - 2;
	pos_y = -1;
	stone = rand() % (sizeof(STONE_DATA) / sizeof(STONE_DATA[0]));
	rotation = rand() % 4;
	drop = 0;
}


void clear_lines(void) {
	int x, y;
	for (y = 0; y < GRID_HEIGHT; y++) {
		for (x = 0; x < GRID_WIDTH; x++) {
			if (cells[y][x] == 0) break;
		}
		if (x == GRID_WIDTH) {
			lines++;
			memmove(cells[1], cells[0], y * sizeof(cells[0]));
			memset(cells[0], 0, sizeof(cells[0]));
		}
	}
}


int update(int dx, int dy, int rot, int fall) {

	if (!falling) {

		int old_rot = rotation;
		rotation = (rotation + rot + 4) % 4;
		if (collision(NULL)) rotation = old_rot;

		pos_x += dx;
		if (collision(NULL)) pos_x -= dx;

		falling = fall;
	}

	if (falling || drop > 50 || dy) {
		drop = 0;
		pos_y++;
		if (collision(NULL)) {
			falling = 0;
			pos_y--;

			int over;
			collision(&over);
			if (over) return 1;

			int i;
			for (i = 0; i < 16; i++) {
				if (STONE_DATA[stone][i] >> rotation & 1) {
					if (pos_y + i / 4 >= 0) {
						cells[pos_y + i / 4][pos_x + i % 4] = 1;
					}
				}
			}

			clear_lines();
			new_stone();
		}
	}
	else drop++;

	return 0;
}

void draw(void) {
	SDL_FillRect(screen, NULL, 0x222222);
	int x, y, i;
	for (i = 0; i < 16; i++) {
		if (STONE_DATA[stone][i] >> rotation & 1) {
			draw_cell(pos_x + i % 4, pos_y + i / 4, 0xffffff);
		}
	}
	for (y = 0; y < GRID_HEIGHT; y++) {
		for (x = 0; x < GRID_WIDTH; x++) {
			if (cells[y][x]) draw_cell(x, y, 0x0000ff);
		}
	}
}



int main(int argc, char** argv) {

	srand(time(NULL));

	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(
		GRID_WIDTH * CELL_SIZE,
		GRID_HEIGHT * CELL_SIZE,
		32, SDL_HWSURFACE | SDL_DOUBLEBUF);


	if (!screen) {
		SDL_Quit();
		return 1;
	}

	SDL_EnableKeyRepeat(100, 50);

	new_stone();

	int running = 1;
	while (running) {

		int dx = 0;
		int dy = 0;
		int rot = 0;
		int fall = 0;

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) running = 0;
				if (event.key.keysym.sym == SDLK_LEFT) dx--;
				if (event.key.keysym.sym == SDLK_RIGHT) dx++;
				if (event.key.keysym.sym == SDLK_DOWN) dy++;
				if (event.key.keysym.sym == SDLK_x) rot--;
				if (event.key.keysym.sym == SDLK_c) rot++;
				if (event.key.keysym.sym == SDLK_SPACE) fall = 1;
				break;

			case SDL_QUIT:
				running = 0;
				break;
			}
		}


		if (update(dx, dy, rot, fall)) running = 0;

		draw();

		SDL_Flip(screen);
		SDL_Delay(10);
	}
	printf("Lines: %d\n", lines);

	SDL_Quit();
	return 0;
}
