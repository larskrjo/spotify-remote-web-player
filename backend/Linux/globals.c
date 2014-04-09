//
//  globals.c
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#include <stdio.h>
#include "globals.h"
#include "container-callbacks.h"
#include "playlist-callbacks.h"

sp_track *g_currenttrack;
sp_playlist *g_playlist;
int g_track_index;

int g_playback_done;
audio_fifo_t g_audiofifo;

/**
 * The callbacks we are interested in for individual playlists.
 */
sp_playlist_callbacks pl_callbacks = {
	.tracks_added = &tracks_added,
	.tracks_removed = &tracks_removed,
	.tracks_moved = &tracks_moved,
	.playlist_renamed = &playlist_renamed,
};

/**
 * The playlist container callbacks
 */
sp_playlistcontainer_callbacks pc_callbacks = {
	.playlist_added = &playlist_added,
	.playlist_removed = &playlist_removed,
	.container_loaded = &container_loaded,
};