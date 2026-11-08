#include "player.h"
#include "palette.h"
#include "display.h"
#include "bullet.h"
#include "input.h"
#include <libgpu.h>
#include <libetc.h>

/* Frames between shots (cooldown). */
#define FIRE_COOLDOWN 5

/* Movement tuning (16.16 fixed-point: 65536 = 1.0 px/frame).
 * ACCEL 0.25, FRICTION 0.25, MAX 4.0 → floaty build-up and coast. */
#define PLAYER_ACCEL   16384*4
#define PLAYER_FRICTION 16384
#define PLAYER_MAX     262144

/* ===== GLOBALS ===== */

Player player;

/* ===== FUNCTIONS ===== */

void init_player() {
    // player.x/y = center of the cube
    player.x = 160 << 16;
    player.y = 120 << 16;
    player.vx = 0;
    player.vy = 0;
    player.hp = 6;
    player.max_hp = 6;
    player.facing = 1;
    player.fire_timer = 0;

    // Initialize the 3D system
    init_3d(SCREEN_W, SCREEN_H);
    // Cube center at the player position.
    init_3d_object(&player.cube, SHAPE_ICOSAHEDRON,
                   player.x, player.y);
    set_3d_palette(&player.cube, &palette_gold);
    set_3d_scale(&player.cube, 700);
}

void update_player() {
    // Input adds acceleration; friction bleeds it off when released.
    if (button_pressed(PADLleft))  player.vx -= PLAYER_ACCEL;
    if (button_pressed(PADLright)) player.vx += PLAYER_ACCEL;
    if (button_pressed(PADLup))    player.vy -= PLAYER_ACCEL;
    if (button_pressed(PADLdown))  player.vy += PLAYER_ACCEL;

    // Friction (only when no input on that axis)
    if (!button_pressed(PADLleft) && !button_pressed(PADLright) && player.vx)
        player.vx -= (player.vx > 0) ? PLAYER_FRICTION : -PLAYER_FRICTION;
    if (!button_pressed(PADLup) && !button_pressed(PADLdown) && player.vy)
        player.vy -= (player.vy > 0) ? PLAYER_FRICTION : -PLAYER_FRICTION;

    // Clamp to max speed
    if (player.vx >  PLAYER_MAX) player.vx =  PLAYER_MAX;
    if (player.vx < -PLAYER_MAX) player.vx = -PLAYER_MAX;
    if (player.vy >  PLAYER_MAX) player.vy =  PLAYER_MAX;
    if (player.vy < -PLAYER_MAX) player.vy = -PLAYER_MAX;

    if (player.vx < 0) player.facing = -1;
    if (player.vx > 0) player.facing =  1;

    player.x += player.vx;
    player.y += player.vy;

    // Pixel position (16.16 → integer)
    int px = player.x >> 16;
    int py = player.y >> 16;

    // Clamp the cube's center to the screen
    if (px < 0)        { px = 0;        player.x = 0; }
    if (px > SCREEN_W / 2) { px = SCREEN_W / 2; player.x = (int)(SCREEN_W / 2) << 16; }
    if (py < 0)        { py = 0;        player.y = 0; }
    if (py > SCREEN_H) { py = SCREEN_H; player.y = (int)SCREEN_H << 16; }

    // Cube center follows the player (1:1 mapping since z == SCREEN_Z)
    move_3d_object(&player.cube, px, py);

    // Advance rotation each frame
    rotate_3d_object(&player.cube, 0, 16, 16);

    // Constant shooting with a cooldown: fire when the timer expires.
    if (player.fire_timer > 0) {
        player.fire_timer--;
    } else {
        fire_player_bullet(px, py);
        player.fire_timer = FIRE_COOLDOWN;
    }

    // Check for enemy bullet hits: each hit removes its bullet and deals damage.
    while (enemy_bullet_collides(px, py, 20)) {
        player.hp--;
        if (player.hp < 0) player.hp = 0;
    }
}

void draw_player() {
    // 3D cube at the player position
    draw_3d_object(&player.cube, db[db_idx].ot, OTSIZE);
}
