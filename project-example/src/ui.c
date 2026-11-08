#include "ui.h"
#include "libgpu.h"
#include "player.h"
#include "display.h"
#include <sys/types.h>

#define MARGIN 16
#define BAR_W (SCREEN_W - 2 * MARGIN)
#define THICKNESS 8
#define DRAIN_STEP 3  /* pixels per frame the bar shrinks */

TILE healthbar;
static int disp_w;      /* current displayed width (px) */
static int hp_to_fixed; /* 65536 / max_hp, computed once (no per-frame div) */

void draw_ui() {
    if (hp_to_fixed == 0) hp_to_fixed = 65536 / player.max_hp;
    int target = (player.hp * hp_to_fixed * BAR_W) >> 16;

    /* Move toward the target a fixed amount per frame (drain on damage,
     * fill on start/heal/restart). */
    if (disp_w > target) {
        disp_w -= DRAIN_STEP;
        if (disp_w < target) disp_w = target;
    } else if (disp_w < target) {
        disp_w += DRAIN_STEP;
        if (disp_w > target) disp_w = target;
    }
    if (disp_w <= 0) return;

    SetTile(&healthbar);
    setXY0(&healthbar, MARGIN, SCREEN_H - MARGIN - THICKNESS);
    setWH(&healthbar, disp_w, THICKNESS);
    setRGB0(&healthbar, 255, 24, 24);
    addPrim(&db[db_idx].ot[0], &healthbar);
}
