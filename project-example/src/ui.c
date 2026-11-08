#include "ui.h"
#include "libgpu.h"
#include "player.h"
#include "display.h"
#include <sys/types.h>

#define UI_PRIMBUF_SIZE 1024
#define MARGIN 16
#define THICKNESS 8

static u_char r, g, b;

static u_char primbuf[2][UI_PRIMBUF_SIZE];
static u_char * next_prim;

TILE healthbar;

void draw_ui() {
	r = 255;
	SetTile(&healthbar);
	setXY0(&healthbar, MARGIN, SCREEN_H - MARGIN);
	setWH(&healthbar, ((float)player.hp/player.max_hp)*(SCREEN_W - 2*MARGIN), THICKNESS);
	setRGB0(&healthbar, r, g, b);
	addPrim(&db[db_idx].ot[0], &healthbar);
}
