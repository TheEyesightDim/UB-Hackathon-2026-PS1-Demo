#pragma once

#include <sys/types.h>
#include <libgpu.h>
#include <stdbool.h>

#include "libgpu.h"

typedef struct Image {
	u_short tpage_idx;
	u_short clut_idx;
	u_char u;
	u_char v;
	u_short w;
	u_short h;
} Image;

// Load a .TIM texture in a buffer to VRAM
void load_texture(u_long * tim_buf, TIM_IMAGE * tim);

void init_image(Image * image, TIM_IMAGE * tim);

// Call this first before drawing with draw_image().
void begin_image_draw();

// Draw a loaded texture as a flat image directly to screen
void draw_image(Image * image, int x, int y, bool is_bg);
