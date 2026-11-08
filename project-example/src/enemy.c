#include "enemy.h"
#include "display.h"


Enemy enemies[MAX_ENEMIES];
int   enemy_count = 0;
TILE tiles[2][MAX_ENEMIES];
short waves = 0;

void init_enemies() {
    enemy_count = MAX_ENEMIES;
    for(short i = 0; i < enemy_count; i++)
    {
        enemies[i].x = SCREEN_W/2 + (i * 10);
        enemies[i].y = -10;
        enemies[i].vx = 0;
        enemies[i].vy = 0;
        enemies[i].enemy_damage = 1;
        enemies[i].enemy_health = 10;
        if(waves > 0){
            enemies[i].enemy_damage = enemies[i].enemy_damage + (2 * waves);
            enemies[i].enemy_health = enemies[i].enemy_health +  waves;
        }
        enemies[i].alive = true;
        enemies[i].timer = i * 120;
        enemies[i].state = WAIT_AT_TOP;
    }
}

void update_enemies (){
    for (short i = 0; i < MAX_ENEMIES; i++){
        if(enemies[i].alive && enemies[i].enemy_health <= 0){
            enemies[i].alive = false;
            enemy_count -= 1;
        }
        if(enemies[i].alive && enemies[i].state == WAIT_AT_TOP){
            if(enemies[i].timer > 0){
                enemies[i].timer -= 1;
            }
            else{
                enemies[i].state = MOVE_DOWN;
                enemies[i].vy = 1;
            }
        }
        if(enemies[i].alive && enemies[i].state == MOVE_DOWN){
            enemies[i].y += enemies[i].vy;
            if(enemies[i].y >= SCREEN_H){
                enemies[i].state = WAIT_AT_BOTTOM;
                enemies[i].vy = 0;
                enemies[i].timer = 120;
            }
        }
        if(enemies[i].alive && enemies[i].state == WAIT_AT_BOTTOM){
            if(enemies[i].timer > 0){
                enemies[i].timer -= 1;
            }
            else{
                enemies[i].state = MOVE_UP;
                enemies[i].vy = -1;
            } 
        }
        if(enemies[i].alive && enemies[i].state == MOVE_UP){
            enemies[i].y += enemies[i].vy;
            if(enemies[i].y <= -10){
                enemies[i].state = WAIT_AT_TOP;
                enemies[i].vy = 0;
                enemies[i].timer = 120;
            }
        }
    }
    if(enemy_count <= 0){
        update_waves();
        init_enemies();
    }
}

void update_waves()
{
    if(enemy_count <= 0){
        waves += 1;
    }
}

void draw_enemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if(enemies[i].alive){
            setTile(&tiles[db_idx][i]);
            setXY0(&tiles[db_idx][i], enemies[i].x, enemies[i].y);
            setWH(&tiles[db_idx][i], 10, 10);
            setRGB0(&tiles[db_idx][i], 255, 0, 0);
            addPrim(&db[db_idx].ot[512], &tiles[db_idx][i]);
        }
    }
}
