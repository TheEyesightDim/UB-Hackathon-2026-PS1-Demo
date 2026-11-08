#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>
#include "3d.h"

/* ===== TYPES ===== */

typedef struct {
    short x, y;       // screen position (center of cube/tile)
    short vx, vy;     // velocity
    short hp;
    short max_hp;
    int   facing;     // 1 = right, -1 = left
    short fire_timer; // frames until next shot (cooldown)

    // 3D cube player (uses the 3d.h module)
    Object3D cube;
} Player;

/* ===== GLOBALS ===== */

extern Player player;

/* ===== FUNCTIONS ===== */

void init_player();
void update_player();
void draw_player();

#endif
