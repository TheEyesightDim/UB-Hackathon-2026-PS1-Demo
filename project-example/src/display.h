#ifndef DISPLAY_H
#define DISPLAY_H

#include <libgpu.h>
#include <libetc.h>

/* ===== PREPROCESSOR ===== */

#define SCREEN_W 320
#define SCREEN_H 240

// Number of depth levels in ordering table
#define OTSIZE 1024

/* ===== TYPES ===== */

typedef struct DoubleBuffer {
	DRAWENV draw;
	DISPENV disp;
	u_long ot[OTSIZE];
} DoubleBuffer;

/* ===== GLOBALS ===== */

// Double-buffered display
extern DoubleBuffer db[2];
extern int db_idx;

/* ===== FUNCTIONS ===== */

void init_display();
void update_display();

#endif
