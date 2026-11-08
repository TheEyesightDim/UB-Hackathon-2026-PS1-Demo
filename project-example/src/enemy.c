#include "enemy.h"
#include "display.h"
#include "bullet.h"
#include "powerup.h"
#include "palette.h"
#include "sfx.h"


Enemy enemies[MAX_ENEMIES];
int   enemy_count = 0;
int   score = 0;
short waves = 0;
short ding_timer = 0;
short wave_banner_timer = 0;

/* Small LCG for random spawn placement. */
static u_long enemy_rng_state = 0x12345678UL;
static u_short enemy_rng(void) {
    enemy_rng_state = enemy_rng_state * 1103515245UL + 12345UL;
    return (u_short)(enemy_rng_state >> 16);
}

/* Speed ramps up gradually: +1 px/frame every 8 waves. */
static short enemy_speed(void) { return 1 + waves / 8; }

void init_enemies() {
    enemy_count = 4 + waves;
    if(enemy_count >= MAX_ENEMIES){
        enemy_count = MAX_ENEMIES;
    }
    /* Clear all slots so leftover enemies from a previous game don't persist. */
    for(short i = 0; i < MAX_ENEMIES; i++){
        enemies[i].alive = false;
    }
    for(short i = 0; i < enemy_count; i++)
    {
        //Intialization of each enemies variables
        // Spawn off-screen (top or bottom) on the right side, 8px off the border.
        enemies[i].x = SCREEN_W / 2 + (enemy_rng() % (SCREEN_W / 2 - 8));
        if(enemy_rng() & 1){
            // Top spawn, entering downward.
            enemies[i].y = -(10 + (enemy_rng() % 40));
            enemies[i].state = MOVE_DOWN;
            enemies[i].vy = enemy_speed();
        } else {
            // Bottom spawn, entering upward.
            enemies[i].y = SCREEN_H + 10 + (enemy_rng() % 40);
            enemies[i].state = MOVE_UP;
            enemies[i].vy = -enemy_speed();
        }
        enemies[i].vx = 0;
        enemies[i].enemy_damage = 1;
        enemies[i].alive = true;
        enemies[i].move_delay = enemy_rng() % 90;  /* stagger entry so they don't clump */
        enemies[i].shoot_timer = 90 + (enemy_rng() % 60);

        /* Harder variants unlock on later waves: cube from wave 2,
         * green pyramid from wave 4. */
        u_short roll = enemy_rng() % 8;
        if(roll < 1 && waves >= 4){
            /* Rare variant: green pyramid, 6 HP, emerald palette, green bullets. */
            enemies[i].score_value = 3;
            enemies[i].enemy_health = 6;
            init_3d_object(&enemies[i].model, SHAPE_PYRAMID, enemies[i].x, enemies[i].y);
            set_3d_scale(&enemies[i].model, 700);
            set_3d_palette(&enemies[i].model, &palette_emerald);
            enemies[i].bullet_r = 120;
            enemies[i].bullet_g = 255;
            enemies[i].bullet_b = 180;
        } else if(roll < 4 && waves >= 2){
            /* Tough variant: gold cube, 4 HP, orange bullets. */
            enemies[i].score_value = 2;
            enemies[i].enemy_health = 4;
            init_3d_object(&enemies[i].model, SHAPE_CUBE, enemies[i].x, enemies[i].y);
            set_3d_scale(&enemies[i].model, 380);
            set_3d_palette(&enemies[i].model, &palette_gold);
            enemies[i].bullet_r = 255;
            enemies[i].bullet_g = 160;
            enemies[i].bullet_b = 64;
        } else {
            enemies[i].score_value = 1;
            enemies[i].enemy_health = 2;
            init_3d_object(&enemies[i].model, SHAPE_DIAMOND, enemies[i].x, enemies[i].y);
            set_3d_scale(&enemies[i].model, 400);
            set_3d_palette(&enemies[i].model, &palette_ruby);
            enemies[i].bullet_r = 255;
            enemies[i].bullet_g = 64;
            enemies[i].bullet_b = 64;
        }
        /* Random initial spin so enemies don't all start aligned. */
        enemies[i].model.rotation.vx = enemy_rng() & 0xFFFF;
        enemies[i].model.rotation.vy = enemy_rng() & 0xFFFF;
        enemies[i].model.rotation.vz = enemy_rng() & 0xFFFF;
    }
    wave_banner_timer = 180;  /* show "WAVE N" for ~3s */
}

void update_enemies (){
    /* New-wave ding: plays shortly after the next wave spawns. */
    if (ding_timer > 0) {
        ding_timer--;
        if (ding_timer <= 0) {
            play_sfx_pitch(SFX_DING, 0x1000);
        }
    }

    /* Wave banner countdown. */
    if (wave_banner_timer > 0) {
        wave_banner_timer--;
    }
    for (short i = 0; i < MAX_ENEMIES; i++){
        if(enemies[i].alive){
            // Random delay before this enemy starts moving/shooting, so
            // they enter the screen at staggered times instead of a clump.
            if(enemies[i].move_delay > 0){
                enemies[i].move_delay -= 1;
            } else if(enemies[i].bounce_delay > 0){
                /* Linger off-screen after a bounce before re-entering. */
                enemies[i].bounce_delay -= 1;
            } else {
                // Boomerang: move vertically, bouncing at the screen edges.
                // On bounce the enemy pops off-screen, pauses briefly,
                // then re-enters from the opposite side.
                enemies[i].y += enemies[i].vy;
                /* Once fully off-screen, bounce: flip direction and pause
                 * before re-entering. The enemy drifts off naturally first. */
                if(enemies[i].vy < 0 && enemies[i].y < -24){
                    enemies[i].vy = enemy_speed();
                    enemies[i].state = MOVE_DOWN;
                    enemies[i].bounce_delay = enemy_rng() % 90;
                } else if(enemies[i].vy > 0 && enemies[i].y > SCREEN_H + 24){
                    enemies[i].vy = -enemy_speed();
                    enemies[i].state = MOVE_UP;
                    enemies[i].bounce_delay = enemy_rng() % 90;
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
                        fire_enemy_bullet(enemies[i].x, enemies[i].y, enemies[i].bullet_r, enemies[i].bullet_g, enemies[i].bullet_b);
                        enemies[i].shoot_timer = 90 - (2 * waves) + (enemy_rng() % 30);
                    }
                    //destorys bullet if it collides with player
                    if(player_bullet_collides(enemies[i].x + 5, enemies[i].y + 5, 20)){
                        enemies[i].enemy_health -= 1;
                        flash_3d_object(&enemies[i].model, FLASH_WHITE, 8); /* white flash on hit */
                    }
                }
            }
            move_3d_object(&enemies[i].model,enemies[i].x,enemies[i].y);
            rotate_3d_object(&enemies[i].model,0,16,16);
            update_3d_flash(&enemies[i].model);
            //kills enemy
            if(enemies[i].alive && enemies[i].enemy_health <= 0){
                enemies[i].alive = false;
                enemy_count -= 1;
                /* Score: base value (1/2/3) x current wave number (1-based). */
                score += enemies[i].score_value * (waves + 1);
                /* Low-chance powerup drop (1/11). */
                if(enemy_rng() % 11 == 0){
                    spawn_powerup(enemies[i].x, enemies[i].y);
                }
            }
        }
    }
    //updates the wave to the next wave and cues the ding
    if(enemy_count <= 0){
        update_waves();
        clear_enemy_bullets();
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

