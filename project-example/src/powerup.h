#ifndef POWERUP_H
#define POWERUP_H

#include <sys/types.h>
#include "3d.h"

#define MAX_POWERUPS 8

/* Powerup kinds (randomized on spawn). */
typedef enum {
    POWERUP_SPLIT,   /* timed split-bullet (3-way) */
    POWERUP_FAST,    /* timed rapid fire */
    POWERUP_HOMING,  /* timed homing bullets */
    POWERUP_TYPE_COUNT
} PowerupType;
/* A torus that drifts left; on collect grants a timed powerup (split or fast). */
typedef struct {
    short x, y;       // screen position (center)
    short vx, vy;     // velocity (px/frame)
    short spin;       // rotation delta (1/256 deg/frame)
    short pulse;      // scale-pulse phase (0..255)
    short alive;      // 1 = active, 0 = free slot
    u_char type;      // PowerupType
    Object3D model;
} Powerup;

extern Powerup powerups[MAX_POWERUPS];
extern int     powerup_count;

void init_powerups();
void clear_powerups();
void update_powerups();
void draw_powerups();

/* Spawn a powerup at (x,y) drifting left. No-op if all slots are in use. */
void spawn_powerup(short x, short y);

#endif
