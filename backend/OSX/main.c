#include <apr.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/thread.h>
#include <getopt.h>
#include <libspotify/api.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <syslog.h>

#include "audio.h"
#include "server.h"
#include "jukebox.h"
#include "helper.h"
#include "globals.h"
#include "ws-callbacks.h"
#include "sp-callbacks.h"
#include "state.h"
#include "requests.h"

extern const unsigned char g_appkey[];
extern const size_t g_appkey_size;

int main(int argc, char **argv) {
    set_thread_name("Main");
    print_thread_name("main");
    // Open syslog
    openlog("spotify-api-server", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    
    // Initialize program state
    struct state *state = malloc(sizeof(struct state));
    
    // Web server defaults
    state->http_host = strdup("127.0.0.1");
    state->http_port = 23456;
    
    // Initialize libev w/ pthreads
    evthread_use_pthreads();
    
    state->event_base = event_base_new();
    state->async = event_new(state->event_base, -1, 0, &process_events, state);
    state->timer = evtimer_new(state->event_base, &process_events, state);
    state->sigint = evsignal_new(state->event_base, SIGINT, &sigint_handler, state);
    state->exit_status = EXIT_FAILURE;
    
    // Initialize mutexes
    pthread_mutex_init(&search_mutex, NULL);
    pthread_mutex_init(&playlist_mutex, NULL);
    pthread_mutex_init(&json_mutex, NULL);

    
    // Initialize APR
    apr_status_t rv = apr_initialize();
    
    if (rv != APR_SUCCESS) {
        syslog(LOG_CRIT, "Unable to initialize APR");
    } else {
        apr_pool_create(&state->pool, NULL);
        
        // Initialize libspotify
        sp_session_callbacks session_callbacks = {
            .logged_in = &logged_in,
            .logged_out = &logged_out,
            .notify_main_thread = &notify_main_thread,
            .music_delivery = &music_delivery,
            .metadata_updated = &metadata_updated,
            .play_token_lost = &play_token_lost,
            .log_message = NULL,
            .end_of_track = &end_of_track,
        };
        
        sp_session_config session_config = {
            .api_version = SPOTIFY_API_VERSION,
            .application_key_size = 0,
            .cache_location = ".cache",  // Cache location is required
            .settings_location = ".settings",
            .callbacks = &session_callbacks,
            .user_agent = "spotify-redlight",
            .userdata = state,
        };
        
        // Parse command line arguments
        char *credentials_blob = NULL;
        int remember_me = false;
        int relogin = false;
        struct option opts[] = {
            // Login configuration
            {"username", required_argument, NULL, 'u'},
            {"password", required_argument, NULL, 'p'},
            {"remember-me", no_argument, &remember_me, 1},
            {"credentials", required_argument, NULL, 'c'},
            {"relogin", no_argument, &relogin, 1},
            
            {"credentials-path", required_argument, NULL, 'k'},
            
            // Application key file (binary) path
            {"application-key", required_argument, NULL, 'A'},
            
            // Session configuration
            {"cache-location", required_argument, NULL, 'C'},
            {"compress-playlists", no_argument,
                (int*)&session_config.compress_playlists, 1},
            {"dont-save_metadata_for_playlists", no_argument,
                (int*)&session_config.dont_save_metadata_for_playlists, 1},
            {"initially-unload_playlists", no_argument,
                (int*)&session_config.initially_unload_playlists, 1},
            {"settings-location", required_argument, NULL, 'S'},
            {"tracefile", required_argument, NULL, 'T'},
            {"user-agent", required_argument, NULL, 'U'},
            
            // HTTP options
            {"host", required_argument, NULL, 'H'},
            {"port", required_argument, NULL, 'P'},
            
            {NULL, 0, NULL, 0}
        };
        const char optstring[] = "u:p:l:c:k:A:C:S:T:U:H:P:";
        
        for (int c; (c = getopt_long(argc, argv, optstring, opts, NULL)) != -1; ) {
            switch (c) {
                case 'u':
                    g_username = strdup(optarg);
                    break;
                    
                case 'p':
                    g_password = strdup(optarg);
                    break;
                    
                case 'l':
                    g_listname = strdup(optarg);
                    break;
                    
                case 'c':
                    credentials_blob = strdup(optarg);
                    break;
                    
                case 'k':
                    state->credentials_blob_filename = strdup(optarg);
                    session_callbacks.credentials_blob_updated = &credentials_blob_updated;
                    break;
                    
                case 'A':
                    read_application_key(optarg, &session_config);
                    break;
                    
                case 'C':
                    session_config.cache_location = strdup(optarg);
                    break;
                    
                case 'S':
                    session_config.settings_location = strdup(optarg);
                    break;
                    
                case 'T':
                    session_config.tracefile = strdup(optarg);
                    break;
                    
                case 'U':
                    session_config.user_agent = strdup(optarg);
                    break;
                    
                case 'H':
                    state->http_host = strdup(optarg);
                    break;
                    
                case 'P':
                    state->http_port = atoi(optarg);
                    break;
                default:
                    break;
            }
        }
        
        if (session_config.application_key_size == 0) {
            fprintf(stderr, "You didn't specify a path to your application key (use"
                    " -A/--application-key).\n");
        } else {
            sp_session *session;
            sp_error session_create_error = sp_session_create(&session_config,
                                                              &session);
            if (session_create_error != SP_ERROR_OK) {
                syslog(LOG_CRIT, "Error creating Spotify session: %s",
                       sp_error_message(session_create_error));
            } else {
                // Log in to Spotify
                if (relogin) {
                    sp_session_relogin(session);
                } else {
                    sp_session_login(session, g_username, g_password, remember_me, credentials_blob);
                }
                audio_init(&g_audiofifo);
                event_base_dispatch(state->event_base);
            }
        }
        if (credentials_blob != NULL) free(credentials_blob);
    }
    
    event_free(state->async);
    event_free(state->timer);
    event_free(state->sigint);
    if (state->http != NULL)
        evhttp_free(state->http);
    free(state->http_host);
    event_base_free(state->event_base);
    int exit_status = state->exit_status;
    free(state);
    return exit_status;
}
