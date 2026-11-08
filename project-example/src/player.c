#include "player.h"
#include "display.h"
#include "input.h"
#include <libgpu.h>
#include <libetc.h>

/* ===== GLOBALS ===== */

Player player;
TILE playerTile;

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
    player.vx = 0;
    player.vy = 0;

    if (button_pressed(PADLleft))  player.vx = -2;
    if (button_pressed(PADLright)) player.vx =  2;
    if (button_pressed(PADLup))    player.vy = -2;
    if (button_pressed(PADLdown))  player.vy =  2;

    if (player.vx < 0) player.facing = -1;
    if (player.vx > 0) player.facing =  1;

    player.x += player.vx;
    player.y += player.vy;

    // Keep the 16x16 tile on screen
    if (player.x < 0)           player.x = 0;
    if (player.x > SCREEN_W-16) player.x = SCREEN_W - 16;
    if (player.y < 0)           player.y = 0;
    if (player.y > SCREEN_H-16) player.y = SCREEN_H - 16;
}

void draw_player() {
    setTile(&playerTile);
    setXY0(&playerTile, player.x, player.y);
    setWH(&playerTile, 16, 16);
    setRGB0(&playerTile, 0, 255, 0);
    addPrim(&db[db_idx].ot[512], &playerTile);
}
