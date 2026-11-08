#ifndef BULLET_H
#define BULLET_H

#include <sys/types.h>
#include <stdbool.h>

#define MAX_PLAYER_BULLETS 32
#define MAX_ENEMY_BULLETS  64
#define BULLET_SPEED 6
#define ENEMY_BULLET_SPEED 2   /* player = 6 */

typedef struct {
    short x, y;       // screen position (center of bullet)
    short vx, vy;     // velocity
    short alive;      // 1 = active, 0 = free slot
    u_char r, g, b;   // bullet color
} Bullet;

/* Two independent pools: player bullets and enemy bullets. */
extern Bullet player_bullets[MAX_PLAYER_BULLETS];
extern Bullet enemy_bullets[MAX_ENEMY_BULLETS];
extern int    player_bullet_count;
extern int    enemy_bullet_count;

void init_bullets();
void clear_enemy_bullets();
void update_bullets();
void draw_bullets();

/* Spawn a player bullet at (x,y) traveling forward (to the right).
 * No-op if all slots are in use. */
void fire_player_bullet(short x, short y);

/* Spawn an enemy bullet at (x,y) traveling forward (to the left)
 * in the given color. No-op if all slots are in use. */
void fire_enemy_bullet(short x, short y, u_char r, u_char g, u_char b);

/* True if a live player bullet is within `threshold` pixels of (x,y).
 * Kills (frees) that bullet on a hit. Uses squared distance (no sqrt). */
bool player_bullet_collides(short x, short y, int threshold);

/* True if a live enemy bullet is within `threshold` pixels of (x,y).
 * Kills (frees) that bullet on a hit. Uses squared distance (no sqrt). */
bool enemy_bullet_collides(short x, short y, int threshold);

#endif
