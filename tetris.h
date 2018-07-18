#include <string.h>


extern const char STONE_DATA[7][16];

enum {
	CELL_SIZE = 20,
	GRID_WIDTH = 10,
	GRID_HEIGHT = 20,
	STONE_COUNT = sizeof(STONE_DATA) / sizeof(STONE_DATA[0])
};


typedef struct Particle Particle;
struct Particle {
	Particle* next;
	int c;
	float x;
	float y;
	float vx;
	float vy;
};


typedef struct {
	int x;
	int y;
	int rot;
	int stone;
	int perm[STONE_COUNT];
	int perm_pos;
	int lines;
	int stones;
	int tick;
	enum { NORMAL, FALLING, BLINK, OVER } state;
	int cells[GRID_HEIGHT][GRID_WIDTH];
	int full_lines[GRID_HEIGHT];
	Particle* particles;
	int quake;
} Grid;


int collision(const Grid* grid, int over);
int bot(Grid* grid, int depth, int* dx, int* dy, int* rot, int* fall);

