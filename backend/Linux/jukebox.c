#include <errno.h>
#include <libgen.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/time.h>
#include <libspotify/api.h>

#include "audio.h"
#include "helper.h"
#include "globals.h"
#include "sp-callbacks.h"


/* --- Data --- */
/// The application key is specific to each project, and allows Spotify
/// to produce statistics on how our service is used.
extern const uint8_t g_appkey[];
/// The size of the application key.
extern const size_t g_appkey_size;
/// Non-zero when a track has ended and the jukebox has not yet started a new one
int g_playback_done;


/**
 * Called on various events to start playback if it hasn't been started already.
 *
 * The function simply starts playing the first track of the playlist.
 */
void try_jukebox_start(void)
{
	sp_track *t;

	if (!g_playlist)
		return;

	if (!sp_playlist_num_tracks(g_playlist)) {
		fprintf(stderr, "jukebox: No tracks in playlist. Waiting\n");
		return;
	}

	if (sp_playlist_num_tracks(g_playlist) < g_track_index) {
		fprintf(stderr, "jukebox: No more tracks in playlist. Waiting\n");
		return;
	}

	t = sp_playlist_track(g_playlist, g_track_index);

	if (g_currenttrack && t != g_currenttrack) {
		/* Someone changed the current track */
		audio_fifo_flush(&g_audiofifo);
		sp_session_player_unload(g_sess);
		g_currenttrack = NULL;
	}

	if (!t)
		return;

	if (sp_track_error(t) != SP_ERROR_OK)
		return;

	if (g_currenttrack == t)
		return;

	g_currenttrack = t;

	printf("jukebox: Now playing \"%s\"...\n", sp_track_name(t));
	fflush(stdout);

	sp_session_player_load(g_sess, t);
	sp_session_player_play(g_sess, 1);
}

/**
 * Changes the track to the given track
 */
void change_track(int index)
{
    g_track_index = (index % sp_playlist_num_tracks(g_playlist));
	if (g_currenttrack) {
		g_currenttrack = NULL;
		sp_session_player_unload(g_sess);
        try_jukebox_start();
	}
}

void load_playlist(sp_error error) {
    sp_playlistcontainer *pc = sp_session_playlistcontainer(g_sess);
    
	if (SP_ERROR_OK != error) {
		fprintf(stderr, "jukebox: Login failed: %s\n",
                sp_error_message(error));
		exit(2);
	}
    
	sp_playlistcontainer_add_callbacks(pc, &pc_callbacks, NULL);
    
	printf("jukebox: Looking at %d playlists\n", sp_playlistcontainer_num_playlists(pc));
    
	for (int i = 0; i < sp_playlistcontainer_num_playlists(pc); ++i) {
		sp_playlist *pl = sp_playlistcontainer_playlist(pc, i);
        
		sp_playlist_add_callbacks(pl, &pl_callbacks, NULL);
        
		if (!strcasecmp(sp_playlist_name(pl), g_listname)) {
			g_playlist = pl;
			try_jukebox_start();
		}
	}
    
	if (!g_playlist) {
		printf("jukebox: No such playlist. Waiting for one to pop up...\n");
		fflush(stdout);
	}
}