#include "player.h"

/* ===== GLOBALS ===== */

Player player;

/* ===== FUNCTIONS ===== */

void init_player() {
    player.x = 160;
    player.y = 200;
    player.vx = 0;
    player.vy = 0;
    player.hp = 6;
    player.max_hp = 6;
    player.facing = 1;
}

void update_player() {
    // TODO: input handling
}

void draw_player() {
    // TODO: player sprite
}
