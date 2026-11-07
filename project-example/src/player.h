#ifndef PLAYER_H
#define PLAYER_H

#include <sys/types.h>

/* ===== TYPES ===== */

typedef struct {
    short x, y;       // screen position
    short vx, vy;     // velocity
    short hp;
    short max_hp;
    int   facing;     // 1 = right, -1 = left
} Player;

/* ===== GLOBALS ===== */

extern Player player;

/* ===== FUNCTIONS ===== */

void init_player();
void update_player();
void draw_player();

#endif
