//
//  search.c
//  webserver
//
//  Created by Lars Kristian Johansen on 06/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <libspotify/api.h>
#include <jansson.h>
#include <event2/http.h>
#include <stdlib.h>

#include "requests.h"
#include "server.h"
#include "helper.h"
#include "json.h"
#include "globals.h"
#include "server.h"

// Search request
struct evhttp_request *s_request;
// Playlist request
struct evhttp_request *p_request;

// Lock to protect search request
pthread_mutex_t search_mutex;
// Lock to protect playlist changes
pthread_mutex_t playlist_mutex;


static void extract_track(int index, json_t* json, sp_track *track, sp_artist* artist)
{
    json_t* track_info = json_array();
    // Track name
    json_array_append_new(track_info, json_string(sp_track_name(track)));
    // Track artist name
    json_array_append_new(track_info, json_string(sp_artist_name(sp_track_artist(track, 0))));
    // Track duration
    json_array_append_new(track_info, json_integer(sp_track_duration(track)));
    // Track relative id
    json_array_append_new(track_info, json_integer(index));
    char* urilink = (char*)malloc(100*sizeof(char));
    int length = sp_link_as_string(sp_link_create_from_track(track, 0), urilink, 100);
    char* spotifyhttp = (char*)malloc((length+16)*sizeof(char));
    memcpy(spotifyhttp, "http://open.spotify.com/track/", 30);
    memcpy(spotifyhttp+30, urilink+14, length-14);
    // Track uri link
    json_array_append_new(track_info, json_string(urilink));
    // Track http link
    json_array_append_new(track_info, json_string(spotifyhttp));
    json_array_append(json, track_info);
    
    json_decref(track_info);
    free(urilink);
    free(spotifyhttp);
}

static void send_results(sp_search *search)
{
    int i;
    
    printf("Query          : %s\n", sp_search_query(search));
    printf("Did you mean   : %s\n", sp_search_did_you_mean(search));
    printf("Tracks in total: %d\n", sp_search_total_tracks(search));
    puts("");
    
    json_t *json = json_array();
    json_array_append_new(json, json_string(sp_search_did_you_mean(search)));
    
    for (i = 0; i < sp_search_num_tracks(search); ++i)
        extract_track(i, json, sp_search_track(search, i), sp_search_artist(search, i));

    send_reply_json(s_request, HTTP_OK, "OK", json);
}

static void SP_CALLCONV search_complete(sp_search *search, void *userdata)
{
    if (sp_search_error(search) == SP_ERROR_OK)
        send_results(search);
    else
        fprintf(stderr, "Failed to search: %s\n",
                sp_error_message(sp_search_error(search)));
    
    sp_search_release(search);
    pthread_mutex_unlock(&search_mutex);
}

/**
 * Request for search
 */
void search_request(struct evhttp_request *req, char* query)
{
    pthread_mutex_lock(&search_mutex);
    s_request = req;
    sp_search_create(g_sess, query, 0, 60, 0, 60, 0, 60, 0, 60, SP_SEARCH_STANDARD, &search_complete, NULL);
}

void start_request(struct evhttp_request *request, char* localId) {
    pthread_mutex_lock(&playlist_mutex);
    int i = atoi(localId);
    change_track(i);
    json_t* node = json_object();
    send_reply_json(request, HTTP_OK, "OK", node);
    pthread_mutex_unlock(&playlist_mutex);

}

/**
 * Request for playlist
 */
void playlist_request(struct evhttp_request *req)
{
    pthread_mutex_lock(&playlist_mutex);
    p_request = req;
    sp_link *playlist_link = sp_link_create_from_string(g_playlist_uri);
    sp_playlist *playlist = sp_playlist_create(g_sess, playlist_link);
    sp_link_release(playlist_link);
    // If loaded
    if (sp_playlist_is_loaded(playlist)) {
        get_playlist_tracks(playlist, p_request, g_sess);
    } else {
        // Wait for playlist to load
        register_playlist_callbacks(playlist, p_request, get_playlist_tracks,
                                    &playlist_state_changed_callbacks,
                                    g_sess);
    }
}

/**
 * Request for removal of song
 */
void remove_request(struct evhttp_request *request, char* localId) {
    pthread_mutex_lock(&playlist_mutex);
    int i = atoi(localId);
    
    sp_link *playlist_link = sp_link_create_from_string(g_playlist_uri);
    sp_playlist *playlist = sp_playlist_create(g_sess, playlist_link);
    int num_tracks = sp_playlist_num_tracks(playlist);
    while(g_track_index == i) {
        g_track_index = rand() % num_tracks;
        if(g_track_index != i) {
            change_track(g_track_index);
            break;
        }
    }
    sp_playlist_remove_tracks(playlist, &i, 1);
    sp_playlist_release(playlist);
    
    json_t* node = json_object();
    send_reply_json(request, HTTP_OK, "OK", node);
    pthread_mutex_unlock(&playlist_mutex);
    
}

/**
 * Request for addition of song
 */
void add_request(struct evhttp_request *request, char* trackUri) {
    pthread_mutex_lock(&playlist_mutex);
    sp_link *playlist_link = sp_link_create_from_string(g_playlist_uri);
    sp_playlist *playlist = sp_playlist_create(g_sess, playlist_link);
    sp_link *song_link = sp_link_create_from_string(trackUri);
    sp_track *track = sp_link_as_track(song_link);
    sp_playlist_add_tracks(playlist, &track, 1, sp_playlist_num_tracks(playlist), g_sess);
    json_t* node = json_object();
    send_reply_json(request, HTTP_OK, "OK", node);
    sp_link_release(playlist_link);
    sp_link_release(song_link);
    pthread_mutex_unlock(&playlist_mutex);
    
}