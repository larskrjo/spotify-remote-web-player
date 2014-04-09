//
//  server_wscallbacks.h
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#ifndef webserver_server_wscallbacks_h
#define webserver_server_wscallbacks_h

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>
#include <event2/thread.h>
#include <event2/util.h>

#include "server.h"

void process_events(evutil_socket_t socket, short what, void *userdata);
void sigint_handler(evutil_socket_t socket, short what, void *userdata);
// Is set active when spotify is logged in
void handle_request(struct evhttp_request *request, void *userdata);

#endif
