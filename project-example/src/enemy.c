#include "enemy.h"
#include "display.h"
#include "bullet.h"
#include "palette.h"
#include "sfx.h"


Enemy enemies[MAX_ENEMIES];
int   enemy_count = 0;
short waves = 0;
short ding_timer = 0;

/* Small LCG for random spawn placement. */
static u_long enemy_rng_state = 0x12345678UL;
static u_short enemy_rng(void) {
    enemy_rng_state = enemy_rng_state * 1103515245UL + 12345UL;
    return (u_short)(enemy_rng_state >> 16);
}

void init_enemies() {
    enemy_count = 4 + (2* waves);
    if(enemy_count >= MAX_ENEMIES){
        enemy_count = MAX_ENEMIES;
    }
    for(short i = 0; i < enemy_count; i++)
    {
        //Intialization of each enemies variables
        // Spawn off-screen (top or bottom) on the right side of the screen.
        enemies[i].x = SCREEN_W / 2 + (enemy_rng() % (SCREEN_W / 2));
        if(enemy_rng() & 1){
            // Top spawn, entering downward.
            enemies[i].y = -(10 + (enemy_rng() % 40));
            enemies[i].state = MOVE_DOWN;
            enemies[i].vy = 1 + waves;
        } else {
            // Bottom spawn, entering upward.
            enemies[i].y = SCREEN_H + 10 + (enemy_rng() % 40);
            enemies[i].state = MOVE_UP;
            enemies[i].vy = -(1 + waves);
        }
        enemies[i].vx = 0;
        enemies[i].enemy_damage = 1;
        enemies[i].enemy_health = 2;
        if(waves > 0){
            enemies[i].enemy_health = enemies[i].enemy_health +  waves;
        }
        enemies[i].alive = true;
        enemies[i].shoot_timer = 60;

        init_3d_object(&enemies[i].model,SHAPE_DIAMOND,enemies[i].x,enemies[i].y);
        set_3d_scale(&enemies[i].model,500);
        set_3d_palette(&enemies[i].model, &palette_ruby);
    }
}

void update_enemies (){
    /* New-wave ding: plays shortly after the next wave spawns. */
    if (ding_timer > 0) {
        ding_timer--;
        if (ding_timer <= 0) {
            play_sfx_pitch(SFX_DING, 0x1000);
        }
    }

    for (short i = 0; i < MAX_ENEMIES; i++){
        if(enemies[i].alive){
            // Boomerang: move vertically, bouncing at the screen edges.
            // No off-screen wait — the direction flips as soon as they
            // reach the top or bottom of the visible screen.
            enemies[i].y += enemies[i].vy;
            if(enemies[i].y <= 0 && enemies[i].vy < 0){
                // Reached the top moving up: reverse and move down.
                enemies[i].y = 0;
                enemies[i].state = MOVE_DOWN;
                enemies[i].vy = 1 + waves;
            } else if(enemies[i].y >= SCREEN_H && enemies[i].vy > 0){
                // Reached the bottom moving down: reverse and move up.
                enemies[i].y = SCREEN_H;
                enemies[i].state = MOVE_UP;
                enemies[i].vy = -(1 + waves);
            }
            //enemy can only shoot when it's on screen
            if(enemies[i].y >= 0 && enemies[i].y <= SCREEN_H - 10){
                //Timer for when the enemy can shoot
                if(enemies[i].shoot_timer > 0){
                    enemies[i].shoot_timer -= 1;
                }
                //allows the enemy to shoot
                //resets timer to wait again
                else{
                    fire_enemy_bullet(enemies[i].x, enemies[i].y);
                    enemies[i].shoot_timer = 60 - (5 * waves);
                }
                //destorys bullet if it collides with player
                if(player_bullet_collides(enemies[i].x + 5, enemies[i].y + 5, 16)){
                    enemies[i].enemy_health -= 1;
                    flash_3d_object(&enemies[i].model, FLASH_WHITE, 8); /* white flash on hit */
                }
            }
            move_3d_object(&enemies[i].model,enemies[i].x,enemies[i].y);
            rotate_3d_object(&enemies[i].model,0,16,16);
            update_3d_flash(&enemies[i].model);
            //kills enemy
            if(enemies[i].alive && enemies[i].enemy_health <= 0){
                enemies[i].alive = false;
                enemy_count -= 1;
            }
        }
    }
    //updates the wave to the next wave and cues the ding
    if(enemy_count <= 0){
        update_waves();
        init_enemies();
        ding_timer = 15;
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
           draw_3d_object(&enemies[i].model,db[db_idx].ot,OTSIZE);
        }
    }
}
