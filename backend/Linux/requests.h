//
//  search.h
//  webserver
//
//  Created by Lars Kristian Johansen on 06/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#ifndef webserver_search_h
#define webserver_search_h

#include <pthread.h>

extern pthread_mutex_t search_mutex;
extern pthread_mutex_t playlist_mutex;

void search_request(struct evhttp_request *request, char* query);
void start_request(struct evhttp_request *request, char* query);
void remove_request(struct evhttp_request *request, char* localId);
void add_request(struct evhttp_request *request, char* spotifyUri);
void playlist_request(struct evhttp_request *request);

#endif
