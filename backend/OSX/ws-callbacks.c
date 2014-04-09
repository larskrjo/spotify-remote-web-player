//
//  server_wscallbacks.c
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <stdbool.h>
#include <libspotify/api.h>
#include "ws-callbacks.h"
#include "state.h"
#include "globals.h"
#include "jukebox.h"
#include "json.h"
#include "requests.h"

#define HTTP_NOTIMPL 501

void process_events(evutil_socket_t socket, short what, void *userdata) {
    struct state *state = userdata;
    event_del(state->timer);
    int timeout = 0;
    if (g_playback_done) {
        sp_link *playlist_link = sp_link_create_from_string(g_playlist_uri);
        sp_playlist *playlist = sp_playlist_create(state->session, playlist_link);
        int num_tracks = sp_playlist_num_tracks(playlist);
        int i = g_track_index;
        while(g_track_index == i) {
            g_track_index = rand() % num_tracks;
            if(g_track_index != i) {
                change_track(g_track_index);
                break;
            }
        }
        g_playback_done = 0;
    }
    do {
        sp_session_process_events(state->session, &timeout);
    } while (timeout == 0);
    state->next_timeout.tv_sec = timeout / 1000;
    state->next_timeout.tv_usec = (timeout % 1000) * 1000;
    evtimer_add(state->timer, &state->next_timeout);
}

void sigint_handler(evutil_socket_t socket, short what, void *userdata) {
    struct state *state = userdata;
    sp_session_logout(state->session);
}

// Is set active when spotify is logged in
void handle_request(struct evhttp_request *request, void *userdata) {
    evhttp_connection_set_timeout(request->evcon, 1);
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Server", "larskrjo@gmail.com");
    
    // Check request method
    int http_method = evhttp_request_get_command(request);
    
    switch (http_method) {
        case EVHTTP_REQ_GET:
        case EVHTTP_REQ_PUT:
        case EVHTTP_REQ_POST:
        case EVHTTP_REQ_DELETE:
            break;
            
        default:
            evhttp_send_error(request, HTTP_NOTIMPL, "Not Implemented");
            return;
    }
    
    struct state *state = userdata;
    sp_session *session = state->session;
    char *uri = evhttp_decode_uri(evhttp_request_get_uri(request));
    
    char *entity = strtok(uri, "/");
    
    if (entity == NULL) {
        evhttp_send_error(request, HTTP_BADREQUEST, "Bad Request");
        free(uri);
        return;
    }
    
    // Handle requests to /user/<user_name>/inbox
    if (strncmp(entity, "user", 4) == 0) {
        char *username = strtok(NULL, "/");
        
        if (username == NULL) {
            evhttp_send_error(request, HTTP_BADREQUEST, "Bad Request");
            free(uri);
            return;
        }
        
        char *action = strtok(NULL, "/");
        handle_user_request(request, action, username, session);
        free(uri);
        return;
    }
    
    // Handle requests to /search/song_phrase
    if (strncmp(entity, "search", 6) == 0) {
        char *query = strtok(NULL, "/");
        search_request(request, query);
        free(uri);
        return;
    }
    
    // Handle requests to /playlist
    if (strncmp(entity, "playlist", 8) == 0) {
        playlist_request(request);
        free(uri);
        return;
    }
    
    // Handle requests to /start/local_id
    if (strncmp(entity, "start", 5) == 0) {
        char *query = strtok(NULL, "/");
        start_request(request, query);
        free(uri);
        return;
    }
    
    // Handle requests to /remove/local_id
    if (strncmp(entity, "remove", 6) == 0) {
        char *query = strtok(NULL, "/");
        remove_request(request, query);
        free(uri);
        return;
    }
    
    // Handle requests to /add/spotify_uri
    if (strncmp(entity, "add", 3) == 0) {
        char *query = strtok(NULL, "/");
        add_request(request, query);
        free(uri);
        return;
    }
    free(uri);
}
