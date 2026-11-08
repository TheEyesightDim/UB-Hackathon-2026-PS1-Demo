#ifndef PLAYER_H
#define PLAYER_H

#include <stdlib.h>
#include "3d.h"

/* ===== TYPES ===== */

typedef struct {
    int x, y;         // screen position (center), 16.16 fixed-point pixels
    int vx, vy;       // velocity, 16.16 fixed-point pixels/frame
    short hp;
    short max_hp;
    int   facing;     // 1 = right, -1 = left
    short fire_timer; // frames until next shot (cooldown)
    short split_timer; // frames of split-bullet powerup remaining
    short fast_timer;  // frames of rapid-fire powerup remaining
    short homing_timer; // frames of homing-bullet powerup remaining
    short spin_x, spin_y; // eased rotation rate (GTE 1/256 deg/frame)
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
