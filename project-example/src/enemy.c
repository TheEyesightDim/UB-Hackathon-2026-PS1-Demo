#include "enemy.h"
#include "display.h"


Enemy enemies[MAX_ENEMIES];
int   enemy_count = 0;
TILE tiles[2][MAX_ENEMIES];

void init_enemies() {
    enemy_count = 1;
    enemies[0].x = 100;
    enemies[0].y = 100;
    enemies[0].vx = 0;
    enemies[0].vy = 0;
    enemies[0].enemy_damage = 1;
    enemies[0].enemy_health = 10;
    enemies[0].alive = 1;
}

void update_enemies (){
    for (short i = 0; i < MAX_ENEMIES; i++)
    {
        if(enemies[i].alive == 1 && enemies[i].enemy_health <= 0){
            enemies[i].alive = 0;
            enemy_count -= 1;
        }
    }
}

void draw_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if(enemies[i].alive == 1){
            setTile(&tiles[db_idx][i]);
            setXY0(&tiles[db_idx][i], enemies[i].x, enemies[i].y);
            setWH(&tiles[db_idx][i], 100, 100);
            setRGB0(&tiles[db_idx][i], 255, 0, 0);
            addPrim(&db[db_idx].ot[512], &tiles[db_idx][i]);
        }
    }
}
