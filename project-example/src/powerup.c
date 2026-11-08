#include "powerup.h"
#include "display.h"
#include "player.h"
#include "sfx.h"
#include "palette.h"

Powerup powerups[MAX_POWERUPS];
int     powerup_count = 0;

/* Tiny LCG for random powerup type (no stdlib on the PSX). */
static u_long powerup_rng_state = 0x9E3779B9UL;
static u_short powerup_rng(void) {
    powerup_rng_state = powerup_rng_state * 1103515245UL + 12345UL;
    return (u_short)(powerup_rng_state >> 16);
}
/* Leftward drift speed (px/frame). */
#define POWERUP_SPEED 2

/* Player collision radius (px). */
#define POWERUP_HIT_RADIUS 20

/* Split-bullet powerup duration (frames). */
#define POWERUP_SPLIT_FRAMES 360

/* Fast-fire powerup duration (frames). */
#define POWERUP_FAST_FRAMES 360

/* Homing-bullet powerup duration (frames). */
#define POWERUP_HOMING_FRAMES 360

/* Scale pulse: base 700, swings ±110 via a 64-entry sine table (phase 0..63). */
#define POWERUP_BASE_SCALE 700
#define POWERUP_PULSE_AMP  110

static const short sine64[64] = {
    0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 106, 112, 117, 122, 125, 126,
    127, 126, 125, 122, 117, 112, 106, 98, 90, 81, 71, 60, 49, 37, 25, 12,
    0, -12, -25, -37, -49, -60, -71, -81, -90, -98, -106, -112, -117, -122,
    -125, -126, -127, -126, -125, -122, -117, -112, -106, -98, -90, -81,
    -71, -60, -49, -37, -25, -12
};

static void init_set(void) {
    for (int i = 0; i < MAX_POWERUPS; i++)
        powerups[i].alive = 0;
    powerup_count = 0;
}

void init_powerups(void) {
    init_set();
}

void clear_powerups(void) {
    init_set();
}

void spawn_powerup(short x, short y) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (powerups[i].alive) continue;
        /* Randomize between the powerup kinds. */
        PowerupType type = (PowerupType)(powerup_rng() % POWERUP_TYPE_COUNT);
        /* Clamp to the screen so it never spawns past the right/bottom edge. */
        if (x > SCREEN_W) x = SCREEN_W;
        if (x < 0)        x = 0;
        if (y > SCREEN_H) y = SCREEN_H;
        if (y < 0)        y = 0;
        powerups[i].x = x;
        powerups[i].y = y;
        powerups[i].vx = -POWERUP_SPEED;
        powerups[i].vy = 0;
        powerups[i].spin = 20;  /* 1/256 deg/frame */
        powerups[i].pulse = 0;  /* start at mid-scale */
        powerups[i].alive = 1;
        powerups[i].type = type;
        init_3d_object(&powerups[i].model, SHAPE_TORUS, x, y);
        set_3d_scale(&powerups[i].model, 700);  /* 0.5x */
        /* All powerups are neon; the type is revealed by effect. */
        set_3d_palette(&powerups[i].model, &palette_neon);
        powerup_count++;
        return;
    }
}

void update_powerups(void) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerups[i].alive) continue;
        powerups[i].x += powerups[i].vx;
        powerups[i].y += powerups[i].vy;

        /* Scale pulse: advance phase, swing ±POWERUP_PULSE_AMP around base. */
        powerups[i].pulse = (short)((powerups[i].pulse + 2) & 63);
        int s = POWERUP_BASE_SCALE + (POWERUP_PULSE_AMP * sine64[powerups[i].pulse]) / 127;
        set_3d_scale(&powerups[i].model, s);

        /* Cull once off the left edge. */
        if (powerups[i].x < -32) {
            powerups[i].alive = 0;
            powerup_count--;
            continue;
        }

        /* Collision with the player (no effect yet — just detects).
         * player.x/y are 16.16 fixed-point; convert to pixels. */
        int px = player.x >> 16;
        int py = player.y >> 16;
        int dx = powerups[i].x - px;
        int dy = powerups[i].y - py;
        int r2 = POWERUP_HIT_RADIUS * POWERUP_HIT_RADIUS;
        if (dx * dx + dy * dy <= r2) {
            /* Grant the timed powerup effect + play sound. */
            switch (powerups[i].type) {
                case POWERUP_FAST:   player.fast_timer   = POWERUP_FAST_FRAMES;   break;
                case POWERUP_HOMING: player.homing_timer = POWERUP_HOMING_FRAMES; break;
                default:             player.split_timer  = POWERUP_SPLIT_FRAMES;  break;
            }
            play_sfx(SFX_POWERUP);
            powerups[i].alive = 0;
            powerup_count--;
            continue;
        }

        move_3d_object(&powerups[i].model, powerups[i].x, powerups[i].y);
        rotate_3d_object(&powerups[i].model, 0, powerups[i].spin, powerups[i].spin / 2);
    }
}

void draw_powerups(void) {
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerups[i].alive) continue;
        draw_3d_object(&powerups[i].model, db[db_idx].ot, OTSIZE);
    }
}
