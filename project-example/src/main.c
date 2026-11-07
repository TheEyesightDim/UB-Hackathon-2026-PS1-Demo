#include "stddef.h"
#include <malloc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <libapi.h>
#include <kernel.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <libpad.h>
#include <libcd.h>

/*
A lot of the setup code here is based on LameGuy64's tutorials,
Nugget examples, and the example in the PS1-Blender-Exporter repo.
The examples at https://github.com/ABelliqueux/nolibgs_hello_worlds
are also a great resource.
The PS1-Blender-Repo example is also heavily leaned upon to make use
of its support for fully textured and animated blender models.
*/

/* ===== PREPROCESSOR ===== */

#define SCREEN_W 320
#define SCREEN_H 240

#define CD_SECTOR_SIZE 2048
#define len_to_sectors(size) ((size + CD_SECTOR_SIZE -1) / CD_SECTOR_SIZE)

// These values are very much debatable, but should work to start with
#define MAX_STACK_SIZE (1 << 15)
#define STACK_END (0x801FFF00 - MAX_STACK_SIZE)
#define MAX_HEAP_SIZE (1 << 20)
#define HEAP_START (STACK_END - MAX_HEAP_SIZE)

// Number of depth levels in ordering table
#define OTSIZE 1024


/* ===== TYPES ===== */
typedef struct DoubleBuffer {
	DRAWENV draw;
	DISPENV disp;
	u_long ot[OTSIZE];
} DoubleBuffer ;

/* ===== FUNCTIONS =====*/

// Combines all initializing funcs into one call
void init_everything();

void init_display();
void update_display();

void init_dbg_font();

void init_controller();

// returns the filesize of the loaded file, or 0 on error
u_long load_file(char * filename);

// void load_tim();

/* ===== GLOBALS ===== */

// Intermediate buffer, primarily for storing data loaded from CD.
// Cast this to different types as necessary.
u_long load_buffer[(1 << 16)/sizeof(u_long)];

// List of files on disc on their sizes and other info
// Needs to be updated on asset file changes.

// Double-buffered display
DoubleBuffer db[2];
int db_idx;



int main (void) {
	// Call all initializers first
	init_everything();

	
	printf("Debug printing to emulator should work!\n");

	// Main loop
	while (true) {
		// This is not used how the 4.7 Documentation says,
		// But it works anyway because of old C jank.
		// Basically it assumes the stream to use for debugging.
		// "Officially" you'd call it like:
		// FntPrint(stream_id, "format string", args...)
		FntPrint("SEE YOU LATER, SPACE COWBOY...");
		

		// Keep this last
		FntFlush(-1);
		update_display();
	}
}

void init_everything() {
	// Use malloc3 version of mem allocation functions
	// cf. Overview, Kernel Library, 2-23
	InitHeap3((void *)HEAP_START, (u_long)MAX_HEAP_SIZE);
	
	init_display();
	init_dbg_font();
}

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

	// Debug font use
	FntOpen(16, 16, 288, 64, 0, 512);
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

void init_dbg_font() {
	// Load to the last texture page
	FntLoad(960, 256);
}


u_long load_file(char * filename) {
	CdlFILE file;
	int cd_read_result;
	u_char cd_control_result[8] = {0};

	CdSearchFile(&file, filename);
	CdControl(CdlSetloc, (u_char *)&file.pos, cd_control_result);
	CdRead(len_to_sectors(file.size),
	       load_buffer,
	       CdlModeSpeed);

	// Block until read finishes
	return !CdReadSync(0, 0) ? file.size : 0;
}


