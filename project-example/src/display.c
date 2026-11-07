#include "display.h"

/* ===== GLOBALS ===== */

// Double-buffered display
DoubleBuffer db[2];
int db_idx;

/* ===== FUNCTIONS ===== */

void init_display() {
	ResetGraph(0);

	// Set the draw environment at each index to be the opposite of how it was for the display environment. The definitions overlap, but in reverse, so every screen flip, the front and back buffers trade places in a cycle.
	SetDefDispEnv(&db[0].disp, 0, 0, SCREEN_W , SCREEN_H);
	SetDefDispEnv(&db[1].disp, 0, SCREEN_H, SCREEN_W , SCREEN_H);

	SetDefDrawEnv(&db[0].draw, 0, SCREEN_H, SCREEN_W, SCREEN_H);
	SetDefDrawEnv(&db[1].draw, 0, 0, SCREEN_W, SCREEN_H);

	// Set clear color and enable bg clear
	setRGB0(&db[0].draw, 37, 150, 190);
	setRGB0(&db[1].draw, 37, 150, 190);
	db[0].draw.isbg = true;
	db[1].draw.isbg = true;

	// Apply environments
	PutDispEnv(&db[0].disp);
	PutDrawEnv(&db[0].draw);
}

void update_display() {
	// Wait for GPU to finish drawing, then wait for VBlank
	DrawSync(0);
	VSync(0);

	// Flip buffers
	db_idx = !db_idx;

	// Apply environments
	PutDispEnv(&db[db_idx].disp);
	PutDrawEnv(&db[db_idx].draw);

	// Enable display
	SetDispMask(1);
}
