#include "player.h"
#include "palette.h"
#include "display.h"
#include "input.h"
#include <libgpu.h>
#include <libetc.h>

/* ===== GLOBALS ===== */

Player player;
TILE playerTile;

/* ===== FUNCTIONS ===== */

void init_player() {
    // player.x/y = center of the cube
    player.x = 160;
    player.y = 120;
    player.vx = 0;
    player.vy = 0;
    player.hp = 6;
    player.max_hp = 6;
    player.facing = 1;

    // Initialize the 3D system
    init_3d(SCREEN_W, SCREEN_H);
    // Cube center at the player position.
    init_3d_object(&player.cube, SHAPE_ICOSAHEDRON,
                   player.x, player.y);
    set_3d_palette(&player.cube, &palette_gold);
    set_3d_scale(&player.cube, 700);
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

    // Clamp the cube's center to the screen
    if (player.x < 0)        player.x = 0;
    if (player.x > SCREEN_W) player.x = SCREEN_W;
    if (player.y < 0)        player.y = 0;
    if (player.y > SCREEN_H) player.y = SCREEN_H;

    // Cube center follows the player (1:1 mapping since z == SCREEN_Z)
    move_3d_object(&player.cube, player.x, player.y);

    // Advance rotation each frame
    rotate_3d_object(&player.cube, 0, 16, 16);
}

void draw_player() {
    // 2D tile centered on the same point
    setTile(&playerTile);
    setXY0(&playerTile, player.x - 8, player.y - 8);
    setWH(&playerTile, 16, 16);
    setRGB0(&playerTile, 0, 255, 0);
    addPrim(&db[db_idx].ot[512], &playerTile);

    // 3D cube at the same position
    draw_3d_object(&player.cube, db[db_idx].ot, OTSIZE);
}
