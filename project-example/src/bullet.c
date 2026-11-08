#include "bullet.h"
#include "display.h"
#include "sfx.h"
#include "enemy.h"

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
/* Integer square root (Newton's method, 32-bit). Far cheaper on the R3000
 * than the software double-precision sqrt() — no FPU, just integer divs. */
static int isqrt(int n) {
    if (n <= 0) return 0;
    int x = n, y = (x + 1) >> 1;
    while (y < x) { x = y; y = (x + n / x) >> 1; }
    return x;
}

/* Homing: steer a bullet toward the nearest live enemy.
 * Blends the velocity 25% toward the target direction per frame, then
 * re-normalizes to the original speed so integer truncation can't decay
 * the velocity to zero (which would freeze the bullet). */
static void homing_steer(Bullet *b) {
    /* Find the nearest live enemy. */
    int best = -1;
    int best_d2 = 0x7FFFFFFF;
    for (int e = 0; e < MAX_ENEMIES; e++) {
        if (!enemies[e].alive) continue;
        int dx = enemies[e].x - b->x;
        int dy = enemies[e].y - b->y;
        int d2 = dx * dx + dy * dy;
        if (d2 < best_d2) { best_d2 = d2; best = e; }
    }
    if (best < 0) return;  /* no enemies to home on */

    int dx = enemies[best].x - b->x;
    int dy = enemies[best].y - b->y;
    int dist = isqrt(best_d2);
    if (dist < 1) dist = 1;

    int speed = isqrt(b->vx * b->vx + b->vy * b->vy);
    if (speed < 1) speed = 1;

    /* Desired direction (normalized to the bullet's speed), blended 25%. */
    int tx = (dx * speed) / dist;
    int ty = (dy * speed) / dist;
    b->vx = (b->vx * 3 + tx) / 4;
    b->vy = (b->vy * 3 + ty) / 4;

    /* Re-normalize to the original speed (truncation fix). */
    int ns = isqrt(b->vx * b->vx + b->vy * b->vy);
    if (ns < 1) {
        /* Velocity fully truncated — restore the desired direction. */
        b->vx = tx;
        b->vy = ty;
    } else {
        b->vx = (b->vx * speed) / ns;
        b->vy = (b->vy * speed) / ns;
    }
}

static void update_set(Bullet *set, int max, int *count) {
    for (int i = 0; i < max; i++) {
        if (!set[i].alive) continue;
        if (set[i].homing) homing_steer(&set[i]);
        set[i].x += set[i].vx;
        set[i].y += set[i].vy;
        // Cull once well off-screen. The y margin is generous so homing
        // bullets can curve past the top/bottom edge before being killed.
        if (set[i].x < -8 || set[i].x > SCREEN_W + 8 ||
            set[i].y < -32 || set[i].y > SCREEN_H + 32) {
            set[i].alive = 0;
            (*count)--;
        }
    }
}

static void draw_set(Bullet *set, int max, TILE *tiles, int is_player) {
    for (int i = 0; i < max; i++) {
        if (!set[i].alive) continue;
        int w, h;
        if (is_player) {
            if (set[i].shape == BULLET_SHAPE_SQUARE) { w = 8;  h = 8; }
            else                                     { w = 14; h = 5; }
        } else {
            w = 10; h = 4;  /* enemy bullets: thin horizontal */
        }
        setTile(&tiles[i]);
        setXY0(&tiles[i], set[i].x - w / 2, set[i].y - h / 2);
        setWH(&tiles[i], w, h);
        setRGB0(&tiles[i], set[i].r, set[i].g, set[i].b);
        addPrim(&db[db_idx].ot[512], &tiles[i]);
    }
}

/* ===== API ===== */

void init_bullets() {
    init_set(player_bullets, MAX_PLAYER_BULLETS, &player_bullet_count);
    init_set(enemy_bullets, MAX_ENEMY_BULLETS, &enemy_bullet_count);
}

void clear_enemy_bullets() {
    init_set(enemy_bullets, MAX_ENEMY_BULLETS, &enemy_bullet_count);
}

/* Shared spawn: find a free player-bullet slot and set its velocity/color/shape. */
static void spawn_player_bullet(short x, short y, short dx, short dy, u_char shape, u_char homing) {
    for (int i = 0; i < MAX_PLAYER_BULLETS; i++) {
        if (player_bullets[i].alive) continue;
        // Spawn just in front of the shooter (forward = right).
        player_bullets[i].x = x + 12;
        player_bullets[i].y = y;
        player_bullets[i].vx = dx;
        player_bullets[i].vy = dy;
        player_bullets[i].r = 225;
        player_bullets[i].g = 240;
        player_bullets[i].b = 255;
        /* Homing bullets are always squares; otherwise use the given shape. */
        player_bullets[i].shape = homing ? BULLET_SHAPE_SQUARE : shape;
        player_bullets[i].homing = homing;
        player_bullets[i].alive = 1;
        player_bullet_count++;
        return;
    }
}

void fire_player_bullet(short x, short y, u_char homing) {
    // Normal player bullet: rectangle.
    spawn_player_bullet(x, y, BULLET_SPEED, 0, BULLET_SHAPE_RECT, homing);
}

void fire_player_bullet_dir(short x, short y, short dx, short dy, u_char homing) {
    // Powerup (split) bullet: square.
    spawn_player_bullet(x, y, dx, dy, BULLET_SHAPE_SQUARE, homing);
}

void fire_enemy_bullet(short x, short y, u_char r, u_char g, u_char b) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (enemy_bullets[i].alive) continue;
        // Spawn just in front of the shooter (forward = left).
        enemy_bullets[i].x = x - 12;
        enemy_bullets[i].y = y;
        enemy_bullets[i].vx = -ENEMY_BULLET_SPEED;
        enemy_bullets[i].vy = 0;
        enemy_bullets[i].r = r;
        enemy_bullets[i].g = g;
        enemy_bullets[i].b = b;
        enemy_bullets[i].shape = BULLET_SHAPE_RECT;
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
    // Per-bullet color/shape; player rect/square, enemy thin horizontal.
    draw_set(player_bullets, MAX_PLAYER_BULLETS, playerBulletTiles, 1);
    draw_set(enemy_bullets, MAX_ENEMY_BULLETS, enemyBulletTiles, 0);
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
