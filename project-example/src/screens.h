#pragma once

/* Game flow states. */
typedef enum {
    SCREEN_START = 0,
    SCREEN_PLAYING,
    SCREEN_GAME_OVER,
} ScreenState;

extern ScreenState screen_state;

/* Sets the state to SCREEN_START. Call once at boot. */
void init_screens();

/* Call once per frame, after update_input() and the game updates.
 * Handles START -> PLAYING (start button) and PLAYING -> GAME_OVER (hp 0).
 * GAME_OVER -> PLAYING restarts via the start button. */
void update_screens();

/* Draw the start or game-over text (FntPrint). Call only when not playing. */
void draw_screens();
