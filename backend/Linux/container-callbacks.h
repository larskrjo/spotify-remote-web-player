//
//  container.h
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#ifndef webserver_container_h
#define webserver_container_h

void playlist_removed(sp_playlistcontainer *pc, sp_playlist *pl, int position, void *userdata);
void playlist_added(sp_playlistcontainer *pc, sp_playlist *pl, int position, void *userdata);
void container_loaded(sp_playlistcontainer *pc, void *userdata);

#endif
