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
        sp_link *playlist_link = sp_link_create_from_string("spotify:user:kurume:playlist:5xGIekf3Gw3S4rDwgQaZWF");
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
    
    
//    // Handle requests to /playlist/<playlist_uri>/<action>
//    if (strncmp(entity, "playlist", 8) != 0) {
//        evhttp_send_error(request, HTTP_BADREQUEST, "Bad Request");
//        free(uri);
//        return;
//    }
//    
//    char *playlist_uri = strtok(NULL, "/");
//    
//    if (playlist_uri == NULL) {
//        switch (http_method) {
//            case EVHTTP_REQ_PUT:
//            case EVHTTP_REQ_POST:
//                put_playlist(NULL, request, session);
//                break;
//                
//            default:
//                send_error(request, HTTP_BADREQUEST, "Bad Request");
//                break;
//        }
//        
//        free(uri);
//        return;
//    }
//    
//    sp_link *playlist_link = sp_link_create_from_string(playlist_uri);
//    
//    if (playlist_link == NULL) {
//        send_error(request, HTTP_NOTFOUND, "Playlist link not found");
//        free(uri);
//        return;
//    }
//    
//    if (sp_link_type(playlist_link) != SP_LINKTYPE_PLAYLIST) {
//        sp_link_release(playlist_link);
//        send_error(request, HTTP_BADREQUEST, "Not a playlist link");
//        free(uri);
//        return;
//    }
//    
//    sp_playlist *playlist = sp_playlist_create(session, playlist_link);
//    sp_link_release(playlist_link);
//    
//    if (playlist == NULL) {
//        send_error(request, HTTP_NOTFOUND, "Playlist not found");
//        free(uri);
//        return;
//    }
//    
//    // Dispatch request
//    char *action = strtok(NULL, "/");
//    
//    // Default request handler
//    handle_playlist_fn request_callback = &not_implemented;
//    void *callback_userdata = session;
//    
//    switch (http_method) {
//        case EVHTTP_REQ_GET:
//        {
//            if (action == NULL) {
//                // Send entire playlist
//                request_callback = &get_playlist;
//            } else if (strncmp(action, "collaborative", 13) == 0) {
//                request_callback = &get_playlist_collaborative;
//            } else if (strncmp(action, "subscribers", 11) == 0) {
//                request_callback = &get_playlist_subscribers;
//            }
//        }
//            break;
//            
//        case EVHTTP_REQ_PUT:
//        case EVHTTP_REQ_POST:
//        {
//            if (strncmp(action, "add", 3) == 0) {
//                request_callback = &put_playlist_add_tracks;
//            } else if (strncmp(action, "remove", 6) == 0) {
//                request_callback = &put_playlist_remove_tracks;
//            } else if (strncmp(action, "patch", 5) == 0) {
//                callback_userdata = state;
//                request_callback = &put_playlist_patch;
//            }
//        }
//            break;
//            
//        case EVHTTP_REQ_DELETE:
//        {
//            callback_userdata = state;
//            request_callback = &delete_playlist;
//        }
//            break;
//    }
//    
//    if (sp_playlist_is_loaded(playlist)) {
//        request_callback(playlist, request, callback_userdata);
//    } else {
//        // Wait for playlist to load
//        register_playlist_callbacks(playlist, request, request_callback,
//                                    &playlist_state_changed_callbacks,
//                                    callback_userdata);
//    }
    free(uri);
}
