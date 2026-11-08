#ifndef ENEMY_H
#define ENEMY_H

#include <sys/types.h>
#include <stdbool.h>

#define MAX_ENEMIES 16

enum State{
    WAIT_AT_TOP = 0,
    MOVE_DOWN = 1,
    WAIT_AT_BOTTOM = 2,
    MOVE_UP = 3
};

typedef struct {
    short x, y;
    short vx, vy;
    short enemy_health, enemy_damage;
    bool alive;
    short timer;
    enum State state;
} Enemy;

extern Enemy enemies[MAX_ENEMIES];
extern int   enemy_count;
extern short waves;

void init_enemies();
void update_enemies();
void update_waves();
void draw_enemies();

#endif
