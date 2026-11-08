#include "enemy.h"
#include "display.h"
#include "bullet.h"


Enemy enemies[MAX_ENEMIES];
int   enemy_count = 0;
TILE tiles[2][MAX_ENEMIES];
short waves = 0;

void init_enemies() {
    enemy_count = MAX_ENEMIES;
    for(short i = 0; i < enemy_count; i++)
    {
        //Intialization of each enemies variables
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
        enemies[i].shoot_timer = 60;

        init_3d_object(&enemies[i].model,SHAPE_DIAMOND,enemies[i].x,enemies[i].y);

    }
}

void update_enemies (){
    for (short i = 0; i < MAX_ENEMIES; i++){
        if(enemies[i].alive){
            //Works when the enemy is at the top
            if(enemies[i].state == WAIT_AT_TOP){
                //the timer for when the enemy can move down
                if(enemies[i].timer > 0){
                    enemies[i].timer -= 1;
                }
                //changes velocity to move down
                else{
                    enemies[i].state = MOVE_DOWN;
                    enemies[i].vy = 2;
                }
            }
            //Works while enemy is moving down 
            if(enemies[i].state == MOVE_DOWN){
                //moves the enemy down
                enemies[i].y += enemies[i].vy;
                //Stops the enemy when it's offscreen
                //resets the velocity
                //resets the wait time
                if(enemies[i].y >= SCREEN_H){
                    enemies[i].state = WAIT_AT_BOTTOM;
                    enemies[i].vy = 0;
                    enemies[i].timer = 120;
                }
            }
            //Works while enemy is waiting at bottom
            if(enemies[i].state == WAIT_AT_BOTTOM){
                //the timer for when the enemy can move up
                if(enemies[i].timer > 0){
                    enemies[i].timer -= 1;
                }
                //changes velocity to move on
                else{
                    enemies[i].state = MOVE_UP;
                    enemies[i].vy = -2;
                } 
            }
            //Works while enemy is moving up
            if(enemies[i].state == MOVE_UP){
                //moves the enemy up
                enemies[i].y += enemies[i].vy;
                //Stops enemy at the top
                //resets the velocity
                // resets the wait time
                if(enemies[i].y <= -10){
                    enemies[i].state = WAIT_AT_TOP;
                    enemies[i].vy = 0;
                    enemies[i].timer = 120;
                }
            }
            //enemy can only shoot when it's on screen
            if((enemies[i].state == MOVE_UP || enemies[i].state == MOVE_DOWN) && (enemies[i].y >= 0 && enemies[i].y <= SCREEN_H - 10)){
                //Timer for when the enemy can shoot
                if(enemies[i].shoot_timer > 0){
                    enemies[i].shoot_timer -= 1;
                }
                //allows the enemy to shoot
                //resets timer to wait again
                else{
                    fire_enemy_bullet(enemies[i].x, enemies[i].y);
                    enemies[i].shoot_timer = 60;
                }
                //destorys bullet if it collides with player
                if(player_bullet_collides(enemies[i].x + 5, enemies[i].y + 5, 8)){
                    enemies[i].enemy_health -= 1;
                }
            }
            //kills enemy 
            if(enemies[i].alive && enemies[i].enemy_health <= 0){
                enemies[i].alive = false;
                enemy_count -= 1;
            }
        }
    }
    //updates the wave to the next wave
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
