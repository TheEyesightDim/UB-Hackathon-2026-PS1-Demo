#include "enemy.h"
#include "display.h"

Enemy enemies[MAX_ENEMIES];
int   enemy_count = 0;
TILE enemyTile;


void init_enemies() {
    enemy_count = 1;
    enemies[0].x = 100;
    enemies[0].y = 100;
    enemies[0].vx = 0;
    enemies[0].vy = 0;
}

void update_enemies() {
    //todo
}

void draw_enemies() {
    for (int i = 0; i < enemy_count; i++) {
        setTile(&enemyTile);
        setXY0(&enemyTile, enemies[i].x, enemies[i].y);
        setWH(&enemyTile, 100, 100);
        setRGB0(&enemyTile, 255, 0, 0);
        addPrim(&db[db_idx].ot[512], &enemyTile);
    }
}
