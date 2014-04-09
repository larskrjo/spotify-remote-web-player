//
//  globals.h
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#ifndef webserver_globals_h
#define webserver_globals_h

#include <libspotify/api.h>
#include "audio.h"

#define MAX_APPLICATION_KEY_SIZE 1024

// Length of track URI
static const int kTrackLinkLength = 512;  // Big enough for (also local) URI
// Length of playlist URI
static const int kPlaylistLinkLength = 512;
// Maximum number of characters in a playlist title
static const int kMaxPlaylistTitleLength = 256;

// Playlist variables
extern sp_track *g_currenttrack;
extern sp_playlist *g_playlist;
extern int g_track_index;

// Control flags
extern int g_playback_done;

// Audio and callbacks
extern audio_fifo_t g_audiofifo;
extern sp_playlist_callbacks pl_callbacks;
extern sp_playlistcontainer_callbacks pc_callbacks;

#endif
