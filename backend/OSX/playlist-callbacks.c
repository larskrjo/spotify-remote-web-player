//
//  playlist-callback.c
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#include <stdio.h>
#include <strings.h>
#include "playlist-callbacks.h"
#include "helper.h"
#include "globals.h"

/**
 * Callback from libspotify. Something renamed the playlist.
 *
 * @param  pl            The playlist handle
 * @param  userdata      The opaque pointer
 */
void playlist_renamed(sp_playlist *pl, void *userdata)
{
    print_thread_name("playlist_renamed");
	const char *name = sp_playlist_name(pl);
    
	if (!strcasecmp(name, g_listname)) {
		g_playlist = pl;
		g_track_index = 0;
		try_jukebox_start();
	} else if (g_playlist == pl) {
		printf("jukebox: current playlist renamed to \"%s\".\n", name);
		g_playlist = NULL;
		g_currenttrack = NULL;
		sp_session_player_unload(g_sess);
	}
}

/**
 * Callback from libspotify, saying that a track has been added to a playlist.
 *
 * @param  pl          The playlist handle
 * @param  tracks      An array of track handles
 * @param  num_tracks  The number of tracks in the \c tracks array
 * @param  position    Where the tracks were inserted
 * @param  userdata    The opaque pointer
 */
void tracks_added(sp_playlist *pl, sp_track * const *tracks,
                  int num_tracks, int position, void *userdata)
{
    print_thread_name("tracks_added");
	if (pl != g_playlist)
		return;
    
	printf("jukebox: %d tracks were added\n", num_tracks);
	fflush(stdout);
	try_jukebox_start();
}

/**
 * Callback from libspotify, saying that a track has been removed from a playlist.
 *
 * @param  pl          The playlist handle
 * @param  tracks      An array of track indices
 * @param  num_tracks  The number of tracks in the \c tracks array
 * @param  userdata    The opaque pointer
 */
void tracks_removed(sp_playlist *pl, const int *tracks,
                    int num_tracks, void *userdata)
{
    print_thread_name("tracks_removed");
	int i, k = 0;
    
	if (pl != g_playlist)
		return;
    
	for (i = 0; i < num_tracks; ++i)
		if (tracks[i] < g_track_index)
			++k;
    
	g_track_index -= k;
    
	printf("jukebox: %d tracks were removed\n", num_tracks);
	fflush(stdout);
	try_jukebox_start();
}

/**
 * Callback from libspotify, telling when tracks have been moved around in a playlist.
 *
 * @param  pl            The playlist handle
 * @param  tracks        An array of track indices
 * @param  num_tracks    The number of tracks in the \c tracks array
 * @param  new_position  To where the tracks were moved
 * @param  userdata      The opaque pointer
 */
void tracks_moved(sp_playlist *pl, const int *tracks,
                  int num_tracks, int new_position, void *userdata)
{
    print_thread_name("tracks_moved");
	if (pl != g_playlist)
		return;
    
	printf("jukebox: %d tracks were moved around\n", num_tracks);
	fflush(stdout);
	try_jukebox_start();
}