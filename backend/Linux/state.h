//
//  state.h
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#ifndef webserver_state_h
#define webserver_state_h

#include <linux/limits.h>
#include <apr.h>
#include <apr_pools.h>

// Application state
struct state {
    sp_session *session;
    
    char *credentials_blob_filename;
    
    struct event_base *event_base;
    struct event *async;
    struct event *timer;
    struct event *sigint;
    struct timeval next_timeout;
    
    struct evhttp *http;
    char *http_host;
    int http_port;
    
    apr_pool_t *pool;
    
    int exit_status;
};

#endif
