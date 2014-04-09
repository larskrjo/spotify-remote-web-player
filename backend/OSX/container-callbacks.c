//
//  container.c
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#include <stdio.h>
#include <libspotify/api.h>
#include <strings.h>
#include "helper.h"
#include "globals.h"
#include "container-callbacks.h"

/**
 * Callback from libspotify, telling us a playlist was removed from the playlist container.
 *
 * This is the place to remove our playlist callbacks.
 *
 * @param  pc            The playlist container handle
 * @param  pl            The playlist handle
 * @param  position      Index of the removed playlist
 * @param  userdata      The opaque pointer
 */
void playlist_removed(sp_playlistcontainer *pc, sp_playlist *pl,
                      int position, void *userdata)
{
    print_thread_name("playlist_removed");
	sp_playlist_remove_callbacks(pl, &pl_callbacks, NULL);
}

/**
 * Callback from libspotify, telling us a playlist was added to the playlist container.
 *
 * We add our playlist callbacks to the newly added playlist.
 *
 * @param  pc            The playlist container handle
 * @param  pl            The playlist handle
 * @param  position      Index of the added playlist
 * @param  userdata      The opaque pointer
 */
void playlist_added(sp_playlistcontainer *pc, sp_playlist *pl,
                    int position, void *userdata)
{
    print_thread_name("playlist_added");
	sp_playlist_add_callbacks(pl, &pl_callbacks, NULL);
    
	if (!strcasecmp(sp_playlist_name(pl), g_listname)) {
		g_playlist = pl;
        g_playlist_uri = (char*)malloc(kPlaylistLinkLength*sizeof(char));
        sp_link *playlist_link = sp_link_create_from_playlist(g_playlist);
        sp_link_as_string(playlist_link, g_playlist_uri, kPlaylistLinkLength);
		try_jukebox_start();
	}
}


/**
 * Callback from libspotify, telling us the rootlist is fully synchronized
 * We just print an informational message
 *
 * @param  pc            The playlist container handle
 * @param  userdata      The opaque pointer
 */
void container_loaded(sp_playlistcontainer *pc, void *userdata)
{
    print_thread_name("container_loaded");
	fprintf(stderr, "jukebox: Rootlist synchronized (%d playlists)\n",
            sp_playlistcontainer_num_playlists(pc));
}
