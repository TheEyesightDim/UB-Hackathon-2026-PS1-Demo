#include "cdaudio.h"
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
#include <libspu.h>

#include "display.h"
#include "loader.h"
#include "input.h"
#include "texture.h"

#include "player.h"
#include "enemy.h"

/*
A lot of the setup code here is based on LameGuy64's tutorials,
Nugget examples, and the example in the PS1-Blender-Exporter repo.
The examples at https://github.com/ABelliqueux/nolibgs_hello_worlds
are also a great resource.
The PS1-Blender-Repo example is also heavily leaned upon to make use
of its support for fully textured and animated blender models.
*/

/* ===== PREPROCESSOR ===== */

#define CD_SECTOR_SIZE 2048
#define len_to_sectors(size) ((size + CD_SECTOR_SIZE -1) / CD_SECTOR_SIZE)

// These values are very much debatable, but should work to start with
#define MAX_STACK_SIZE (1 << 15)
#define STACK_END (0x801FFF00 - MAX_STACK_SIZE)
#define MAX_HEAP_SIZE (1 << 20)
#define HEAP_START (STACK_END - MAX_HEAP_SIZE)

/* ===== FUNCTIONS =====*/

// Combines all initializing funcs into one call
void init_everything();

void spu_init();


// void load_tim();

SpuCommonAttr spu_settings;

/* ===== GLOBALS ===== */

int main (void) {
	// Call all initializers first
	init_everything();

	
	printf("Debug printing to emulator should work!\n");

	u_long fsize = load_file("\\DATA\\FSTEST.TIM;1");
	TIM_IMAGE test_image;
	load_texture(load_buffer, &test_image);

	play_track(2);

	// Main loop
	while (true) {
		ClearOTagR(db[db_idx].ot, OTSIZE);
		FntOpen(16, 16, 288, 64, 0, 512);
		// This is not used how the 4.7 Documentation says,
		// But it works anyway because of old C jank.
		// Basically it assumes the stream to use for debugging.
		// "Officially" you'd call it like:
		// FntPrint(stream_id, "format string", args...)
		FntPrint("SEE YOU LATER, SPACE COWBOY...\n");

		// TEST for correct loading of CD files 
		FntPrint("Filesize loaded: %d\n",fsize);

		update_input();
		update_enemies();
		draw_enemies();

		update_player();
		draw_player();

		// Keep this last
		DrawOTag(&db[db_idx].ot[OTSIZE - 1]);
		FntFlush(-1);
		update_display();
	}
}

void init_dbg_font() {
	// Load to the last texture page
	FntLoad(960, 256);
}

void init_everything() {
	// Use malloc3 version of mem allocation functions
	// cf. Overview, Kernel Library, 2-23
	InitHeap3((void *)HEAP_START, (u_long)MAX_HEAP_SIZE);
	
	init_display();
	CdInit();
	spu_init();
	init_dbg_font();

	init_input();
	init_enemies();
	init_player();
}


void spu_init() {
	SpuInit();
	spu_settings.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX);

	// Master volume (between 0 and 0x3fff)
	spu_settings.mvol.left = spu_settings.mvol.right = 0x3fff;

	// CD Volume (0 and 0x7fff)
	spu_settings.cd.volume.left = spu_settings.cd.volume.right = 0x7fff;

	spu_settings.cd.mix = SPU_ON;

	SpuSetCommonAttr(&spu_settings);

	SpuSetTransMode(SPU_TRANSFER_BY_DMA);
}
