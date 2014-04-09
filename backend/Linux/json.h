//
//  json.h
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#ifndef webserver_json_h
#define webserver_json_h

json_t *playlist_to_json(sp_playlist *, json_t *);
json_t *playlistsongs_to_json(sp_playlist *playlist, json_t *object);

json_t *playlist_to_json_only_collaborative(sp_playlist *, json_t *);

// Read track URI into Spotify track
bool json_to_track(json_t *json, sp_track **track);

// Read tracks from an JSON array of track URIs
int json_to_tracks(json_t *json, sp_track **tracks, int num_tracks);

#endif
