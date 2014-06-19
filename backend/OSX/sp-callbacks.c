//
//  server_callbacks.c
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#include <stdio.h>
#include <apr.h>
#include <event2/event.h>
#include <event2/http.h>
#include <libspotify/api.h>
#include <jansson.h>
#include "sp-callbacks.h"
#include "state.h"
#include "helper.h"
#include "jukebox.h"
#include "ws-callbacks.h"
#include "queue.h"
#include "globals.h"


void logged_in(sp_session *session, sp_error error) {
    print_thread_name("logged_in");
    g_sess = session;
    // Binds localhost:1337 to socket, using handle_request for requests.
    bind_webserver(error);
    // Loads the playlist
    load_playlist(error);
}

void logged_out(sp_session *session) {
    print_thread_name("logged_out");
    struct state *state = sp_session_userdata(session);
    event_del(state->async);
    event_del(state->timer);
    event_del(state->sigint);
    event_base_loopbreak(state->event_base);
    apr_pool_destroy(state->pool);
}

void notify_main_thread(sp_session *session) {
    print_thread_name("notify_main_thread");
    // Send event in ws-callbacks to process_events -> sp_session_process_events
    struct state *state = sp_session_userdata(session);
    event_active(state->async, 0, 1);
}

/**
 * This callback is used from libspotify whenever there is PCM data available.
 *
 * @sa sp_session_callbacks#music_delivery
 */
int music_delivery(sp_session *sess, const sp_audioformat *format,
                   const void *frames, int num_frames)
{
//    print_thread_name("music_delivery");
	audio_fifo_t *af = &g_audiofifo;
	audio_fifo_data_t *afd;
	size_t s;
    
	if (num_frames == 0)
		return 0; // Audio discontinuity, do nothing
    
	pthread_mutex_lock(&af->mutex);
    
	/* Buffer one second of audio */
	if (af->qlen > format->sample_rate) {
		pthread_mutex_unlock(&af->mutex);
        
		return 0;
	}
	s = num_frames * sizeof(int16_t) * format->channels;
    
	afd = malloc(sizeof(*afd) + s);
	memcpy(afd->samples, frames, s);
    
	afd->nsamples = num_frames;
    
	afd->rate = format->sample_rate;
	afd->channels = format->channels;
    
	TAILQ_INSERT_TAIL(&af->q, afd, link);
	af->qlen += num_frames;
    
	pthread_cond_signal(&af->cond);
	pthread_mutex_unlock(&af->mutex);
    
	return num_frames;
}

/**
 * Callback called when libspotify has new metadata available
 *
 * @sa sp_session_callbacks#metadata_updated
 */
void metadata_updated(sp_session *sess)
{
	try_jukebox_start();
}

void credentials_blob_updated(sp_session *session, const char *blob) {
    print_thread_name("credentials_blob_updated");
    struct state *state = sp_session_userdata(session);
    if (state->credentials_blob_filename == NULL) {
        return;
    }
    FILE *fp = fopen(state->credentials_blob_filename, "w+");
    if (!fp)
        return;
    size_t blob_size = strlen(blob);
    fwrite(blob, 1, blob_size, fp);
    fclose(fp);
}

/**
 * Notification that some other connection has started playing on this account.
 * Playback has been stopped.
 *
 * @sa sp_session_callbacks#play_token_lost
 */
void play_token_lost(sp_session *sess)
{
	audio_fifo_flush(&g_audiofifo);
    
	if (g_currenttrack != NULL) {
		sp_session_player_unload(g_sess);
		g_currenttrack = NULL;
	}
}

/**
 * This callback is used from libspotify when the current track has ended
 *
 * @sa sp_session_callbacks#end_of_track
 */
void end_of_track(sp_session *sess)
{
	g_playback_done = 1;
}
