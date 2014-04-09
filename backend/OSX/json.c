#include <assert.h>
#include <jansson.h>
#include <libspotify/api.h>
#include <string.h>
#include <stdbool.h>

#include "json.h"
#include "globals.h"

json_t *playlist_to_json_only_collaborative(sp_playlist *playlist,
                                           json_t *object) {
  bool collaborative = sp_playlist_is_collaborative(playlist);
  json_object_set_new(object, "collaborative",
                      collaborative ? json_true() : json_false());
  return object;
}

json_t *playlist_to_json(sp_playlist *playlist, json_t *object) {
  assert(sp_playlist_is_loaded(playlist));

  // Owner
  sp_user *owner = sp_playlist_owner(playlist);
  const char *username = sp_user_display_name(owner);
  sp_user_release(owner);
  json_object_set_new_nocheck(object, "creator",
                              json_string_nocheck(username));

  // URI
  char playlist_uri[kPlaylistLinkLength];
  sp_link *playlist_link = sp_link_create_from_playlist(playlist);

  if (playlist_link == NULL)  // Shouldn't happen; playlist is loaded (?)
    return object;

  sp_link_as_string(playlist_link, playlist_uri, kPlaylistLinkLength);
  sp_link_release(playlist_link);
  json_object_set_new(object, "uri",
                      json_string_nocheck(playlist_uri));

  // Title
  const char *title = sp_playlist_name(playlist);
  json_object_set_new(object, "title",
                      json_string_nocheck(title));

  // Collaborative
  playlist_to_json_only_collaborative(playlist, object);

  // Description
  const char *description = sp_playlist_get_description(playlist);

  if (description != NULL) {
    json_object_set_new(object, "description",
                        json_string_nocheck(description));
  }

  // Number of subscribers
  int num_subscribers = sp_playlist_num_subscribers(playlist);
  json_object_set_new(object, "subscriberCount",
                      json_integer(num_subscribers));

  // Tracks
  json_t *tracks = json_array();
  json_object_set_new(object, "tracks", tracks);
  char track_uri[kTrackLinkLength];

  for (int i = 0; i < sp_playlist_num_tracks(playlist); i++) {
    sp_track *track = sp_playlist_track(playlist, i);
    sp_link *track_link = sp_link_create_from_track(track, 0);
    sp_link_as_string(track_link, track_uri, kTrackLinkLength);
    json_array_append_new(tracks, json_string_nocheck(track_uri));
    sp_link_release(track_link);
  }
  return object;
}

json_t *playlistsongs_to_json(sp_playlist *playlist, json_t *object) {
    assert(sp_playlist_is_loaded(playlist));
    // Tracks
    json_t *tracks = json_array();
    json_object_set_new(object, "tracks", tracks);
    for (int i = 0; i < sp_playlist_num_tracks(playlist); i++) {
        json_t *track_info = json_array();
        sp_track *track = sp_playlist_track(playlist, i);
        json_t* empty = json_string("empty");
        // Track name
        json_t* name = json_string(sp_track_name(track));
        json_array_append_new(track_info, name==NULL?empty:name);
        // Track artist name
        name = json_string(sp_artist_name(sp_track_artist(track, 0)));
        json_array_append_new(track_info, name==NULL?empty:name);
        // Track length
        json_array_append_new(track_info, json_integer(sp_track_duration(track)));
        // Track relative id
        json_array_append_new(track_info, json_integer(i));
        // Track currently playing
        json_array_append_new(track_info, json_integer(g_track_index==i?1:0));
        char* urilink = (char*)malloc(100*sizeof(char));
        int length = sp_link_as_string(sp_link_create_from_track(track, 0), urilink, 100);
        char* spotifyhttp = (char*)malloc((length+16)*sizeof(char));
        memcpy(spotifyhttp, "http://open.spotify.com/track/", 30);
        memcpy(spotifyhttp+30, urilink+14, length-14);
        // Track uri link
        json_array_append_new(track_info, json_string(urilink));
        // Track http link
        json_array_append_new(track_info, json_string(spotifyhttp));
        
        // Add track to array
        json_array_append_new(tracks, track_info);
    }
    return object;
}


bool json_to_track(json_t *json, sp_track **track) {
  if (!json_is_string(json))
    return false;

  sp_link *link = sp_link_create_from_string(json_string_value(json));

  if (link == NULL)
    return false;

  if (sp_link_type(link) != SP_LINKTYPE_TRACK) {
    sp_link_release(link);
    return false;
  }

  *track = sp_link_as_track(link);
  return *track != NULL;
}

int json_to_tracks(json_t *json, sp_track **tracks, int num_tracks) {
  if (!json_is_array(json))
    return 0;

  int num_valid_tracks = 0;

  for (int i = 0; i < num_tracks; i++) {
    json_t *item = json_array_get(json, i);

    if (json_to_track(item, &tracks[num_valid_tracks]))
      num_valid_tracks++;
  }

  return num_valid_tracks;
}
