#ifndef INPUT_H
#define INPUT_H

#include <sys/types.h>
#include <stdbool.h>
#include <libetc.h>   /* PAD* button bit defines */

/* ===== FUNCTIONS ===== */

/* Call once at startup. */
void init_input();

/* Call once per frame before querying buttons. */
void update_input();

/* True if the given button is held down this frame.
 * Pass a PAD* define from libetc.h, e.g. button_held(PADLleft). */
bool button_held(u_long b);

/* True only on the frame the button transitions from up to down. */
bool button_just_pressed(u_long b);

/* Kept for compatibility: same as button_held. */
#define button_pressed(b) button_held(b)

/* Raw button state from the last update_input() call. */
u_long buttons_held();

#endif
