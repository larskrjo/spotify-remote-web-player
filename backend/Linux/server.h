//
//  server.h
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#ifndef webserver_server_h
#define webserver_server_h

#include <linux/limits.h>
#include <apr.h>
#include <event2/event.h>
#include <libspotify/api.h>
#include <jansson.h>

#include "helper.h"

#define HTTP_ERROR 500

typedef void (*handle_playlist_fn)(sp_playlist *playlist, struct evhttp_request *request, void *userdata);

extern sp_playlist_callbacks playlist_state_changed_callbacks;
extern pthread_mutex_t json_mutex;

void not_implemented(sp_playlist *playlist, struct evhttp_request *request, void *userdata);
void get_playlist(sp_playlist *playlist, struct evhttp_request *request, void *userdata);
void get_playlist_tracks(sp_playlist *playlist, struct evhttp_request *request, void *userdata);
void get_playlist_collaborative(sp_playlist *playlist, struct evhttp_request *request, void *userdata);
void get_playlist_subscribers(sp_playlist *playlist, struct evhttp_request *request, void *userdata);
void put_playlist_add_tracks(sp_playlist *playlist, struct evhttp_request *request, void *userdata);
void put_playlist_remove_tracks(sp_playlist *playlist, struct evhttp_request *request, void *userdata);
void put_playlist_patch(sp_playlist *playlist, struct evhttp_request *request, void *userdata);
void delete_playlist(sp_playlist *playlist, struct evhttp_request *request, void *userdata);

void handle_user_request(struct evhttp_request *request, char *action, const char *canonical_username, sp_session *session);
void put_playlist(sp_playlist *playlist, struct evhttp_request *request, void *userdata);
struct playlist_handler* register_playlist_callbacks(sp_playlist *playlist, struct evhttp_request *request, handle_playlist_fn callback, sp_playlist_callbacks *playlist_callbacks, void *userdata);
void send_error(struct evhttp_request *request, int code, const char *message);
void send_reply_json(struct evhttp_request *request, int code, const char *message, json_t *json);

#endif