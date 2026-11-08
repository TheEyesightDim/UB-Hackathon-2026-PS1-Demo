#include "cdaudio.h"
#include <libspu.h>
#include <libcd.h>


static int audio_tracks[2];

void play_track(u_long track_no) {
	audio_tracks[0] = track_no;
	audio_tracks[1] = 0;
	CdPlay(2, audio_tracks, 0);
}
