#include <stdio.h>
#include <string.h>
#include <time.h>
#include <SDL/SDL.h>

enum {
	CELL_SIZE = 20,
	GRID_WIDTH = 10,
	GRID_HEIGHT = 20,
};


typedef struct {
	int pos_x;
	int pos_y;
	int rotation;
	int stone;
	int lines;
	int tick;
	enum { NORMAL, FALLING, BLINK } state;
	int cells[GRID_HEIGHT][GRID_WIDTH];
	int full_lines[GRID_HEIGHT];
} Grid;


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


void draw_cell(int x, int y, uint32_t border, uint32_t fill) {
	SDL_Rect rect = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
	SDL_FillRect(screen, &rect, border);
	rect.x = x * CELL_SIZE + 1;
	rect.y = y * CELL_SIZE + 1;
	rect.w = rect.h = CELL_SIZE - 2;
	SDL_FillRect(screen, &rect, fill);
}


int collision(const Grid* grid, int over) {
	int i;
	for (i = 0; i < 16; i++) {
		if (STONE_DATA[grid->stone][i] >> grid->rotation & 1) {
			int x = grid->pos_x + i % 4;
			int y = grid->pos_y + i / 4;
			if (x < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT) return 1;
			if (y >= 0 && grid->cells[y][x]) return 1;
			else if (over && grid->cells[y][x]) return 1;
		}
	}
	return 0;
}


void new_stone(Grid* grid) {
	grid->pos_x = GRID_WIDTH / 2 - 2;
	grid->pos_y = -1;
	grid->stone = rand() % (sizeof(STONE_DATA) / sizeof(STONE_DATA[0]));
	grid->rotation = rand() % 4;
}


int update(Grid* grid, int dx, int dy, int rot, int fall) {

	if (grid->state == NORMAL) {

		int old_rot = grid->rotation;
		grid->rotation = (grid->rotation + rot + 4) % 4;
		if (collision(grid, 0)) grid->rotation = old_rot;

		grid->pos_x += dx;
		if (collision(grid, 0)) grid->pos_x -= dx;

		grid->tick++;

		if (fall) grid->state = FALLING;
	}

	if (grid->state == FALLING
	|| (grid->state == NORMAL && (grid->tick > 50 || dy))) {
		grid->tick = 0;
		grid->pos_y++;
		if (collision(grid, 0)) {
			grid->state = NORMAL;
			grid->pos_y--;

			if (collision(grid, 1)) return 1;

			int x, y, i;
			for (i = 0; i < 16; i++) {
				if (STONE_DATA[grid->stone][i] >> grid->rotation & 1) {
					if (grid->pos_y + i / 4 >= 0) {
						grid->cells[grid->pos_y + i / 4][grid->pos_x + i % 4] = 1;
					}
				}
			}

			for (y = 0; y < GRID_HEIGHT; y++) {
				for (x = 0; x < GRID_WIDTH; x++) {
					if (grid->cells[y][x] == 0) break;
				}
				grid->full_lines[y] = 0;
				if (x == GRID_WIDTH) {
					grid->full_lines[y] = 1;
					grid->lines++;
					grid->state = BLINK;
				}
			}
			if (grid->state != BLINK) new_stone(grid);
		}
	}

	if (grid->state == BLINK) {
		if (++grid->tick > 30) {
			grid->tick = 0;
			grid->state = NORMAL;
			new_stone(grid);

			int y;
			for (y = 0; y < GRID_HEIGHT; y++) {
				if (grid->full_lines[y]) {
					memmove(grid->cells[1], grid->cells[0], y * sizeof(grid->cells[0]));
					memset(grid->cells[0], 0, sizeof(grid->cells[0]));
				}
			}
		}
	}

	return 0;
}


void draw(const Grid* grid) {
	int x, y, i;
	if (grid->state != BLINK) {
		for (i = 0; i < 16; i++) {
			if (STONE_DATA[grid->stone][i] >> grid->rotation & 1) {
				draw_cell(grid->pos_x + i % 4, grid->pos_y + i / 4, 0x777777, 0xaaaaaa);
			}
		}
	}
	for (y = 0; y < GRID_HEIGHT; y++) {
		if (grid->state == BLINK && grid->full_lines[y]) {
			if (grid->tick % 14 < 7) {
				for (x = 0; x < GRID_WIDTH; x++) draw_cell(x, y, 0xdddddd, 0xffffff);
			}
		}
		else {
			for (x = 0; x < GRID_WIDTH; x++) {
				if (grid->cells[y][x]) draw_cell(x, y, 0x0000aa, 0x0000ff);
			}
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


	Grid grid;
	memset(&grid, 0, sizeof(grid));
	new_stone(&grid);

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

		if (update(&grid, dx, dy, rot, fall)) running = 0;

		SDL_FillRect(screen, NULL, 0x222222);
		draw(&grid);
		SDL_Flip(screen);
		SDL_Delay(10);
	}
	printf("Lines: %d\n", grid.lines);

	SDL_Quit();
	return 0;
}
