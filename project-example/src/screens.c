#include "screens.h"
#include "display.h"
#include "input.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include <libgpu.h>

ScreenState screen_state = SCREEN_START;

void init_screens() {
    screen_state = SCREEN_START;
}

static void reset_game() {
    waves = 0;
    ding_timer = 0;
    init_bullets();
    init_enemies();
    init_player();
}

void update_screens() {
    if (screen_state == SCREEN_START || screen_state == SCREEN_GAME_OVER) {
        if (button_just_pressed(PADstart)) {
            reset_game();
            screen_state = SCREEN_PLAYING;
        }
    } else if (screen_state == SCREEN_PLAYING) {
        if (player.hp <= 0) {
            screen_state = SCREEN_GAME_OVER;
        }
    }
}

void draw_screens() {
    /* Reopen the font window lower and taller so the screen text
     * fills the middle of the display. 288px / 16px = 18 chars per line;
     * lines below are space-padded to center within that width. */
    FntOpen(16, 32, 288, 176, 0, 512);

    if (screen_state == SCREEN_START) {
        FntPrint("\n\n"
                 "   PSYQ BLASTER\n"
                 "\n\n"
                 "   PRESS START\n"
                 "     TO PLAY\n\n");
    } else if (screen_state == SCREEN_GAME_OVER) {
        FntPrint("\n\n"
                 "    GAME OVER\n"
                 "\n\n"
                 "   PRESS START\n"
                 "    TO RESTART\n\n");
    } else if (screen_state == SCREEN_PLAYING && wave_banner_timer > 0) {
        /* Centered "WAVE N" banner, shown for a few seconds on each new wave. */
        FntPrint("\n\n\n   WAVE %d\n\n", waves + 1);
    }
}
