#include <limits.h>
#include <stdlib.h>
#include "tetris.h"



int rate_grid(Grid* grid) {

	int x, y;
	int magic = 0;

	// holes
	for (x = 0; x < GRID_WIDTH; x++) {
		int b = 0;
		for (y = 0; y < GRID_HEIGHT; y++) {
			if (grid->cells[y][x]) {
				magic--;
				b = 1;
			}
			else if (b) {
				magic -= 25;
			}
		}
	}

	// slopes
	int h = 0;
	for (x = 0; x < GRID_WIDTH + 1; x++) {
		for (y = 0; x < GRID_WIDTH && y < GRID_HEIGHT; y++) {
			if (grid->cells[y][x]) break;
		}
		int d = abs(y - h);
		magic -= d;
		if (d > 2) magic -= (d - 2);
		h = y;
	}


	// height
	int height = 0;
	for (y = GRID_HEIGHT - 1; y >= 0; y--) {
		for (x = 0; x < GRID_WIDTH; x++) {
			if (grid->cells[y][x]) height = GRID_HEIGHT - y - 1;
		}
	}
	magic -= height * 2;


	return magic;
}






int bot(Grid* grid, int depth, int* dx, int* dy, int* rot, int* fall) {
	int r, i, s, x, y;
	Grid tmp_grid;

	if (depth == 0) {
		*dx = 0;
		*dy = 0;
		*rot = 0;
		*fall = 0;
	}

	int magic = INT_MIN;

	int save_rot = grid->rot;
	for (r = 0; r < 4; r++) {
		if (collision(grid, 0)) break;

		int save_x = grid->x;
		int dir = grid->stones & 1 ? -1 : 1;

		while (!collision(grid, 0)) grid->x -= dir;
		grid->x += dir;

		while (!collision(grid, 0)) {
			int save_y = grid->y;

			while (!collision(grid, 0)) grid->y++;
			grid->y--;
			if (!collision(grid, 1)) {
				memcpy(&tmp_grid, grid, sizeof(Grid));


				// transfer stone to grid
				for (i = 0; i < 16; i++) {
					if (STONE_DATA[grid->stone][i] >> grid->rot & 1) {
						if (grid->y + i / 4 >= 0) {
							tmp_grid.cells[grid->y + i / 4][grid->x + i % 4] = 1;
						}
					}
				}

				// remove full lines
				for (y = 0; y < GRID_HEIGHT; y++) {
					for (x = 0; x < GRID_WIDTH; x++) {
						if (tmp_grid.cells[y][x] == 0) break;
					}
					if (x == GRID_WIDTH) {
						memmove(tmp_grid.cells[1], tmp_grid.cells[0], y * sizeof(grid->cells[0]));
						memset(tmp_grid.cells[0], 0, sizeof(grid->cells[0]));
					}
				}

				int m;
				if (depth < 1) {
//				if (0) {
					m = INT_MAX;
					tmp_grid.x = GRID_WIDTH / 2 - 2;
					tmp_grid.y = -1;
					for (s = 0; s < STONE_COUNT; s++) {
						int q = bot(&tmp_grid, depth + 1, NULL, NULL, NULL, NULL);
						if (q < m) m = q;
						tmp_grid.stone = (tmp_grid.stone + 1) % STONE_COUNT;
					}
				}
				else {
					m = rate_grid(&tmp_grid);
				}
				if (m > magic) {
					magic = m;
					if (depth == 0) {
						*dx = (grid->x > save_x) - (grid->x < save_x);
						*rot = save_rot != grid->rot;
						*fall = grid->x - save_x == 0 && save_rot == grid->rot;
						*dy = 1;
					}
				}

			}
			grid->y = save_y;
			grid->x += dir;
		}
		grid->x = save_x;
		grid->rot = (grid->rot + 1) % 4;
	}
	grid->rot = save_rot;

	return magic;
}



