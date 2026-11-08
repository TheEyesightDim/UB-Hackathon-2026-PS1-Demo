#include "input.h"
#include <libetc.h>

/* ===== GLOBALS ===== */

static u_long g_buttons = 0;

/* ===== FUNCTIONS ===== */

void init_input() {
    PadInit(0);
}

void update_input() {
    g_buttons = PadRead(0);
}

bool button_pressed(u_long b) {
    return (g_buttons & b) != 0;
}

u_long buttons_held() {
    return g_buttons;
}
