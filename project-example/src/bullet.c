#include "bullet.h"
#include "display.h"
#include "sfx.h"

/* ===== GLOBALS ===== */

Bullet player_bullets[MAX_PLAYER_BULLETS];
Bullet enemy_bullets[MAX_ENEMY_BULLETS];
int    player_bullet_count = 0;
int    enemy_bullet_count = 0;
TILE   playerBulletTiles[MAX_PLAYER_BULLETS];
TILE   enemyBulletTiles[MAX_ENEMY_BULLETS];

/* ===== INTERNAL ===== */

static void init_set(Bullet *set, int max, int *count) {
    for (int i = 0; i < max; i++)
        set[i].alive = 0;
    *count = 0;
}

static void update_set(Bullet *set, int max, int *count) {
    for (int i = 0; i < max; i++) {
        if (!set[i].alive) continue;
        set[i].x += set[i].vx;
        set[i].y += set[i].vy;
        // Cull once off-screen (collision handled by the caller).
        if (set[i].x < -8 || set[i].x > SCREEN_W + 8 ||
            set[i].y < -8 || set[i].y > SCREEN_H + 8) {
            set[i].alive = 0;
            (*count)--;
        }
    }
}

static void draw_set(Bullet *set, int max, TILE *tiles, int r, int g, int b) {
    for (int i = 0; i < max; i++) {
        if (!set[i].alive) continue;
        setTile(&tiles[i]);
        setXY0(&tiles[i], set[i].x - 6, set[i].y - 2);
        setWH(&tiles[i], 12, 4);
        setRGB0(&tiles[i], r, g, b);
        addPrim(&db[db_idx].ot[512], &tiles[i]);
    }
}

/* ===== API ===== */

void init_bullets() {
    init_set(player_bullets, MAX_PLAYER_BULLETS, &player_bullet_count);
    init_set(enemy_bullets, MAX_ENEMY_BULLETS, &enemy_bullet_count);
}

void fire_player_bullet(short x, short y) {
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (player_bullets[i].alive) continue;
        // Spawn just in front of the shooter (forward = right).
        player_bullets[i].x = x + 12;
        player_bullets[i].y = y;
        player_bullets[i].vx = BULLET_SPEED;
        player_bullets[i].vy = 0;
        player_bullets[i].alive = 1;
        player_bullet_count++;
        return;
    }
}

void fire_enemy_bullet(short x, short y) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (enemy_bullets[i].alive) continue;
        // Spawn just in front of the shooter (forward = left).
        enemy_bullets[i].x = x - 12;
        enemy_bullets[i].y = y;
        enemy_bullets[i].vx = -ENEMY_BULLET_SPEED;
        enemy_bullets[i].vy = 0;
        enemy_bullets[i].alive = 1;
        enemy_bullet_count++;
        return;
    }
}

void update_bullets() {
    update_set(player_bullets, MAX_PLAYER_BULLETS, &player_bullet_count);
    update_set(enemy_bullets, MAX_ENEMY_BULLETS, &enemy_bullet_count);
}

void draw_bullets() {
    // Player bullets: yellow. Enemy bullets: red.
    draw_set(player_bullets, MAX_PLAYER_BULLETS, playerBulletTiles, 255, 255, 128);
    draw_set(enemy_bullets, MAX_ENEMY_BULLETS, enemyBulletTiles, 255, 64, 64);
}

bool player_bullet_collides(short x, short y, int threshold) {
    int thr2 = threshold * threshold;
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (!player_bullets[i].alive) continue;
        int dx = player_bullets[i].x - x;
        int dy = player_bullets[i].y - y;
        if (dx * dx + dy * dy <= thr2) {
            player_bullets[i].alive = 0;
            play_sfx(SFX_BOOM);
            return true;
        }
    }
    return false;
}

bool enemy_bullet_collides(short x, short y, int threshold) {
    int thr2 = threshold * threshold;
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!enemy_bullets[i].alive) continue;
        int dx = enemy_bullets[i].x - x;
        int dy = enemy_bullets[i].y - y;
        if (dx * dx + dy * dy <= thr2) {
            enemy_bullets[i].alive = 0;
            return true;
        }
    }
    return false;
}
