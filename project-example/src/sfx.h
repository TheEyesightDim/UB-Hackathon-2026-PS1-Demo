#pragma once

#include <sys/types.h>

typedef enum {
	SFX_BOOM,
	SFX_HURT,
	SFX_DING,
	SFX_COUNT
} SFX_ID;

void init_sfx(void);
void play_sfx(SFX_ID id);
void play_sfx_pitch(SFX_ID id, u_short pitch);
void stop_sfx(SFX_ID id);
void stop_all_sfx(void);
