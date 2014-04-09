//
//  helper.h
//  jukebox
//
//  Created by Lars Kristian Johansen on 02/04/14.
//
//

#ifndef jukebox_statics_h
#define jukebox_statics_h

#include <pthread.h>
#include <libspotify/api.h>
#include "sp-callbacks.h"

extern const char* g_username;
extern const char* g_password;
extern const char* g_listname;
extern char* g_playlist_uri;
extern sp_session *g_sess;

void print_thread_name(char* func);
void set_thread_name(char* name);
void get_time(struct timespec *ts);
void bind_webserver(sp_error error);
void read_application_key(char *path, sp_session_config *session_config);

#endif
