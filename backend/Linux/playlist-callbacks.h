//
//  playlist-callbacks.h
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#ifndef webserver_playlist_callbacks_h
#define webserver_playlist_callbacks_h

#include <libspotify/api.h>

void playlist_renamed(sp_playlist *pl, void *userdata);
void tracks_added(sp_playlist *pl, sp_track * const *tracks,
                  int num_tracks, int position, void *userdata);
void tracks_removed(sp_playlist *pl, const int *tracks,
                    int num_tracks, void *userdata);
void tracks_moved(sp_playlist *pl, const int *tracks,
                  int num_tracks, int new_position, void *userdata);

#endif
