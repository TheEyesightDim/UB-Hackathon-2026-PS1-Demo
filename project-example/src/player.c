#include "player.h"
#include "palette.h"
#include "display.h"
#include "sfx.h"
#include "input.h"
#include "bullet.h"
#include <libgpu.h>
#include <libetc.h>

/* Frames between shots (cooldown). */
#define FIRE_COOLDOWN 15
#define FIRE_COOLDOWN_FAST 5  /* rapid-fire powerup cooldown */

/* Movement tuning (16.16 fixed-point: 65536 = 1.0 px/frame).
 * ACCEL 0.25, FRICTION 0.25, MAX 4.0 → floaty build-up and coast. */
#define PLAYER_ACCEL   16384*4
#define PLAYER_FRICTION 16384
#define PLAYER_MAX     196608  /* 3.0 px/frame (was 3.5) */

/* ===== GLOBALS ===== */

Player player;

/* ===== FUNCTIONS ===== */

void init_player() {
    // player.x/y = center of the cube
    player.x = 80 << 16;
    player.y = 120 << 16;
    player.vx = 0;
    player.vy = 0;
    player.hp = 6;
    player.max_hp = 6;
    player.facing = 1;
    player.fire_timer = 0;
    player.split_timer = 0;
    player.fast_timer = 0;
    player.homing_timer = 0;
    player.spin_x = 0;
    player.spin_y = 0;

    // Initialize the 3D system
    init_3d(SCREEN_W, SCREEN_H);
    // Cube center at the player position.
    init_3d_object(&player.cube, SHAPE_ICOSAHEDRON,
                   player.x, player.y);
    set_3d_palette(&player.cube, &palette_diamond);
    set_3d_scale(&player.cube, 500);
}

/* Eased spin axis: snaps to `target` when it's stronger (start/reverse),
 * otherwise coasts down on its own (1.5%/frame, min 1 step). This lets the
 * spin outlive the velocity, so it visibly decelerates instead of stopping
 * the instant the player's friction brings velocity to zero. */
static short ease_spin(short cur, int target) {
    if (target != 0 && (target > 0 ? target > cur : target < cur)) return (short)target;
    if (cur > 2)      { int s = cur >> 6;  if (s < 1) s = 1; return (short)(cur - s); }
    if (cur < -2)     { int s = -cur >> 6; if (s < 1) s = 1; return (short)(cur + s); }
    return 0;
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

    // Rotation follows movement: spin rate scales with speed and its
    // direction follows the movement direction (16.16 → GTE 1/256-deg via
    // >>12, so speed 1.0 ≈ the old constant 16). The spin snaps to the
    // target when it's stronger (start/reverse) and coasts down on its own
    // when the velocity fades, so it visibly decelerates.
    int tx = player.vy >> 12;   // vertical move → tumble around X
    int ty = player.vx >> 12;   // horizontal move → spin around Y
    player.spin_x = ease_spin(player.spin_x, tx);
    player.spin_y = ease_spin(player.spin_y, ty);
    rotate_3d_object(&player.cube, player.spin_x, player.spin_y, 0);

    // Constant shooting with a cooldown: fire when the timer expires.
    if (player.fire_timer > 0) {
        player.fire_timer--;
    } else {
        if(button_pressed(PADRup) || button_pressed(PADRright) || button_pressed(PADRdown) || button_pressed(PADRleft)){
            u_char homing = (player.homing_timer > 0) ? 1 : 0;
            if (player.split_timer > 0) {
                /* Split-bullet powerup: straight + two angled shots. */
                fire_player_bullet_dir(px, py, BULLET_SPEED,  0, homing);
                fire_player_bullet_dir(px, py, BULLET_SPEED, -3, homing);
                fire_player_bullet_dir(px, py, BULLET_SPEED,  3, homing);
            } else {
                fire_player_bullet(px, py, homing);
            }
            play_sfx(SFX_SHOOT);
            player.fire_timer = (player.fast_timer > 0) ? FIRE_COOLDOWN_FAST : FIRE_COOLDOWN;
        }
    }

    /* Powerup countdowns. */
    if (player.split_timer > 0) {
        player.split_timer--;
    }
    if (player.fast_timer > 0) {
        player.fast_timer--;
    }
    if (player.homing_timer > 0) {
        player.homing_timer--;
    }

    // Check for enemy bullet hits: each hit removes its bullet and deals damage.
    while (enemy_bullet_collides(px, py, 10)) {
        player.hp--;
        if (player.hp < 0) player.hp = 0;
        play_sfx(SFX_HURT);
        flash_3d_object(&player.cube, FLASH_RAINBOW, 30); /* rainbow flash on hit */
    }

    // Advance any active color flash (rainbow sweep, restores palette)
    update_3d_flash(&player.cube);
}

void draw_player() {
    // 3D cube at the player position
    draw_3d_object(&player.cube, db[db_idx].ot, OTSIZE);
}
