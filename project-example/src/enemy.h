#ifndef ENEMY_H
#define ENEMY_H

#include <sys/types.h>
#include <stdbool.h>
#include "3d.h"

#define MAX_ENEMIES 32

/* Vertical boomerang direction (no off-screen wait states). */
enum State{
    MOVE_DOWN = 0,
    MOVE_UP = 1
};

typedef struct {
    short x, y;
    short vx, vy;
    short enemy_health, enemy_damage;
    bool alive;
    enum State state;
    short move_delay;       /* frames before this enemy starts moving */
    short shoot_timer;
    u_char bullet_r, bullet_g, bullet_b;  /* color of this enemy's bullets */
    Object3D model;
} Enemy;

extern Enemy enemies[MAX_ENEMIES];
extern int   enemy_count;
extern short waves;
extern short ding_timer;   /* frames until the new-wave ding plays */
extern short wave_banner_timer; /* frames left to show the "WAVE N" banner */
void init_enemies();
void update_enemies();
void update_waves();
void draw_enemies();

#endif
