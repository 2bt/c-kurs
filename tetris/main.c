#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "tetris.h"

const char STONE_DATA[7][16] = {
	{0x0,0xa,0x0,0x0,0x5,0xf,0x5,0x5,0x0,0xa,0x0,0x0,0x0,0xa}, // I
	{0x0,0x0,0x0,0x0,0x0,0xf,0xf,0x0,0x0,0xf,0xf}, // O
	{0x4,0x5,0x8,0x0,0xa,0xf,0xa,0x0,0x2,0x5,0x1}, // L
	{0x2,0x5,0x4,0x0,0xa,0xf,0xa,0x0,0x1,0x5,0x8}, // J
	{0x0,0xe,0x0,0x0,0x7,0xf,0xd,0x0,0x0,0xb,0x0}, // T
	{0x0,0x0,0xa,0x0,0x5,0xf,0xa,0x0,0x0,0xf,0x5}, // Z
	{0xa,0x0,0x0,0x0,0xa,0xf,0x5,0x0,0x5,0xf,0x0}, // S
//	{0x9,0x5,0x3,0x0,0xa,0xf,0xa,0x0,0xc,0x5,0x6}, // big T
};


SDL_Renderer* renderer;


void draw_cell(int x, int y, uint32_t border, uint32_t fill) {
/*
	SDL_Rect rect = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };
	SDL_FillRect(screen, &rect, border);
	rect.x = x * CELL_SIZE + 1;
	rect.y = y * CELL_SIZE + 1;
	rect.w = rect.h = CELL_SIZE - 2;
	SDL_FillRect(screen, &rect, fill);
*/
	static SDL_Rect rect = { 0, 0, CELL_SIZE, CELL_SIZE };
	rect.x = x * CELL_SIZE;
	rect.y = y * CELL_SIZE;
	SDL_SetRenderDrawColor(renderer,
		(fill >> 16) & 255,
		(fill >> 8) & 255,
		(fill & 255), 255);


	SDL_RenderFillRect(renderer, &rect);

	SDL_SetRenderDrawColor(renderer,
		(border >> 16) & 255,
		(border >> 8) & 255,
		(border & 255), 255);
	SDL_RenderDrawRect(renderer, &rect);
}



int collision(const Grid* grid, int over) {
	int i;
	for (i = 0; i < 16; i++) {
		if (STONE_DATA[grid->stone][i] >> grid->rot & 1) {
			int x = grid->x + i % 4;
			int y = grid->y + i / 4;
			if (x < 0 || x >= GRID_WIDTH || y >= GRID_HEIGHT) return 1;
			if (y >= 0 && grid->cells[y][x]) return 1;
			if (y < 0 && over) return 1;
		}
	}
	return 0;
}





void new_stone(Grid* grid) {
	grid->stones++;
	grid->x = GRID_WIDTH / 2 - 2;
	grid->y = -1;
	grid->rot = rand() % 4;

	// knuth shuffle
	if (--grid->perm_pos < 0) {
		grid->perm_pos = STONE_COUNT - 1;
		int i;
		for (i = 0; i < STONE_COUNT; i++) grid->perm[i] = i;
		for (i = 0; i < STONE_COUNT; i++) {
			int j = rand() % STONE_COUNT;
			int a = grid->perm[i];
			grid->perm[i] = grid->perm[j];
			grid->perm[j] = a;

		}
	}
	grid->stone = grid->perm[grid->perm_pos];
}


void update(Grid* grid, int dx, int dy, int rot, int fall) {

	if (grid->state == NORMAL) {

		int old_rot = grid->rot;
		grid->rot = (grid->rot + rot + 4) % 4;
		if (collision(grid, 0)) grid->rot = old_rot;

		grid->x += dx;
		if (collision(grid, 0)) grid->x -= dx;

		grid->tick++;

		if (fall) grid->state = FALLING;
	}

	if (grid->state == FALLING
	|| (grid->state == NORMAL && (grid->tick > 50 || dy))) {
		grid->tick = 0;
		grid->y++;
		if (collision(grid, 0)) {
			grid->state = NORMAL;
			grid->y--;

			if (collision(grid, 1)) {
				grid->state = OVER;
				return;
			}

			int x, y, i;
			for (i = 0; i < 16; i++) {
				if (STONE_DATA[grid->stone][i] >> grid->rot & 1) {
					if (grid->y + i / 4 >= 0) {
						grid->cells[grid->y + i / 4][grid->x + i % 4] = 1;
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
}


void draw(const Grid* grid) {
	int x, y, i;
	if (grid->state == NORMAL
	||	grid->state == FALLING) {
		for (i = 0; i < 16; i++) {
			if (STONE_DATA[grid->stone][i] >> grid->rot & 1) {
				draw_cell(grid->x + i % 4, grid->y + i / 4, 0x777777, 0xaaaaaa);
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

	Grid grid;

	// init grid
	memset(&grid, 0, sizeof(grid));
	new_stone(&grid);
	grid.stones = 0;

/*
	// bot test
	int dx = 0;
	int dy = 0;
	int rot = 0;
	int fall = 0;
	while (1) {
		if (grid.state == NORMAL) bot(&grid, 0, &dx, &dy, &rot, &fall);
		update(&grid, dx, dy, rot, fall);
		if (grid.state == OVER || grid.lines > 1000) {
			printf("%8d\n", grid.lines);

			memset(&grid, 0, sizeof(grid));
			new_stone(&grid);
			grid.stones = 0;
		}
	}
//*/
	SDL_Window* window;
	SDL_CreateWindowAndRenderer(
		GRID_WIDTH * CELL_SIZE,
		GRID_HEIGHT * CELL_SIZE,
		0, &window, &renderer);


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
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) running = 0;
				if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) dx--;
				if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) dx++;
				if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) dy++;
				if (event.key.keysym.scancode == SDL_SCANCODE_X) rot--;
				if (event.key.keysym.scancode == SDL_SCANCODE_C) rot++;
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) fall = 1;
				break;

			case SDL_QUIT:
				running = 0;
				break;
			}
		}

		// bot
//		if (grid.state == NORMAL) bot(&grid, 0, &dx, &dy, &rot, &fall);


		update(&grid, dx, dy, rot, fall);
		if (grid.state == OVER) running = 0;


		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		draw(&grid);

		SDL_RenderPresent(renderer);
		SDL_Delay(10);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);


	printf("Stones: %d\n", grid.stones);
	printf("Lines: %d\n", grid.lines);

	return 0;
}

