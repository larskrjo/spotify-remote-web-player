//
//  server_callback.h
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#ifndef webserver_server_callback_h
#define webserver_server_callback_h

#include "jukebox.h"

void logged_in(sp_session *session, sp_error error);
void logged_out(sp_session *session);
void notify_main_thread(sp_session *session);
int music_delivery(sp_session *sess, const sp_audioformat *format, const void *frames, int num_frames);
void metadata_updated(sp_session *sess);
void play_token_lost(sp_session *sess);
void end_of_track(sp_session *sess);
void credentials_blob_updated(sp_session *session, const char *blob);

#endif
