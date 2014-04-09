#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "globals.h"
#include "helper.h"
#include "state.h"
#include "ws-callbacks.h"

const char* g_username = NULL;
const char* g_password = NULL;
const char* g_listname = NULL;
char* g_playlist_uri = NULL;
sp_session *g_sess = NULL;

int get_current_seconds();

void print_thread_name(char* func) {
    char* name = (char*)malloc(40*sizeof(char));
    pthread_getname_np(pthread_self(), name, 40);
    printf("%d [%s] %s\n", get_current_seconds(), name, func);
    free(name);
}

void set_thread_name(const char* name) {
    pthread_setname_np(pthread_self(), name);
}

int get_current_seconds() {
    struct timespec ts;
    get_time(&ts);
    return ((ts.tv_sec*1000000+ts.tv_nsec)/1000)%10000000;
}

void get_time(struct timespec *ts) {
#if _POSIX_TIMERS > 0
    clock_gettime(CLOCK_REALTIME, ts);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    TIMEVAL_TO_TIMESPEC(&tv, ts);
#endif
}

void bind_webserver(sp_error error) {
    struct state *state = sp_session_userdata(g_sess);
    
    if (error != SP_ERROR_OK) {
        state->exit_status = EXIT_FAILURE;
        logged_out(g_sess);
        return;
    }
    state->session = g_sess;
    
    evsignal_add(state->sigint, NULL);
    state->http = evhttp_new(state->event_base);
    evhttp_set_gencb(state->http, &handle_request, state);
    
    // Bind HTTP server
    int bind = evhttp_bind_socket(state->http, state->http_host,
                                  state->http_port);
    
    if (bind == -1) {
        sp_session_logout(g_sess);
        return;
    }
}

// Read application key from file (binary), placing the results into the
// session configuration. Fails silently, without warning, if something
// goes wrong: libspotify will fail later, too.
void read_application_key(char *path, sp_session_config *session_config) {
    struct stat st;
    
    if (stat(path, &st) != 0) {
        return;
    }
    
    int appkey_size = (int)st.st_size;
    
    if (appkey_size > MAX_APPLICATION_KEY_SIZE) {
        // Application key file looks awfully large
        appkey_size = MAX_APPLICATION_KEY_SIZE;
    }
    
    size_t size = sizeof(unsigned char);
    unsigned char *appkey = malloc(appkey_size * size);
    
    if (appkey == NULL) {
        return;
    }
    
    FILE *file = fopen(path, "rb");
    
    if (!file) {
        free(appkey);
        return;
    }
    
    session_config->application_key_size = fread(appkey, size, appkey_size, file);
    session_config->application_key = appkey;
    fclose(file);
}
