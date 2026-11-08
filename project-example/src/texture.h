#pragma once

#include <libgpu.h>

// Load a .TIM texture in a buffer to VRAM
void load_texture(u_long * tim_buf, TIM_IMAGE * tparams);
