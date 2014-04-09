//
//  jukebox.h
//  webserver
//
//  Created by Lars Kristian Johansen on 02/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#ifndef webserver_jukebox_h
#define webserver_jukebox_h

#include "audio.h"

void* process_jukebox_events(void* v);

int music_delivery(sp_session *sess, const sp_audioformat *format,
                          const void *frames, int num_frames);
void metadata_updated(sp_session *sess);
void play_token_lost(sp_session *sess);
void end_of_track(sp_session *sess);

void playlist_added(sp_playlistcontainer *pc, sp_playlist *pl,
                    int position, void *userdata);
void playlist_removed(sp_playlistcontainer *pc, sp_playlist *pl,
                      int position, void *userdata);
void container_loaded(sp_playlistcontainer *pc, void *userdata);
void try_jukebox_start(void);
void change_track(int index);
void load_playlist(sp_error error);


#endif
