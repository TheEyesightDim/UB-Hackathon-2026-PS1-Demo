#include "input.h"
#include <libetc.h>

/* ===== GLOBALS ===== */

static u_long g_buttons     = 0;  /* current frame */
static u_long g_prev_buttons = 0; /* previous frame */

/* ===== FUNCTIONS ===== */

void init_input() {
    PadInit(0);
}

void update_input() {
    g_prev_buttons = g_buttons;
    g_buttons = PadRead(0);
}

bool button_held(u_long b) {
    return (g_buttons & b) != 0;
}

bool button_just_pressed(u_long b) {
    return (g_buttons & b) && !(g_prev_buttons & b);
}

u_long buttons_held() {
    return g_buttons;
}
