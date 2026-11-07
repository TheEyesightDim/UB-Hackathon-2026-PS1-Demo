#ifndef ENEMY_H
#define ENEMY_H

#include <sys/types.h>

#define MAX_ENEMIES 16

typedef struct {
    short x, y;
    short vx, vy;
} Enemy;

extern Enemy enemies[MAX_ENEMIES];
extern int   enemy_count;

void init_enemies();
void draw_enemies();
void update_enemies();

#endif
