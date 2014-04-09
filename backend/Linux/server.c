#include <assert.h>
#include <event2/http.h>
#include <jansson.h>
#include <stdbool.h>
#include <syslog.h>

#include "globals.h"
#include "json.h"
#include "server.h"
#include "jukebox.h"
#include "state.h"
#include "ws-callbacks.h"
#include "requests.h"

#define HTTP_PARTIAL 210
#define HTTP_NOTIMPL 501

// State of a request as it's threaded through libspotify callbacks
struct playlist_handler {
    sp_playlist_callbacks *playlist_callbacks;
    struct evhttp_request *request;
    handle_playlist_fn callback;
    void *userdata;
};

typedef void (*handle_playlistcontainer_fn)(sp_playlistcontainer *, struct evhttp_request *, void *);

struct playlistcontainer_handler {
    sp_playlistcontainer_callbacks *playlistcontainer_callbacks;
    struct evhttp_request *request;
    handle_playlistcontainer_fn callback;
    void *userdata;
};

static void send_reply(struct evhttp_request *request,
                       int code,
                       const char *message,
                       struct evbuffer *body) {
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Content-type", "application/json; charset=UTF-8");
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Access-Control-Allow-Origin", "*");
    bool empty_body = body == NULL;
    
    if (empty_body)
        body = evbuffer_new();
    
    evhttp_send_reply(request, code, message, body);
    
    if (empty_body)
        evbuffer_free(body);
}

pthread_mutex_t json_mutex;

// Sends JSON to the client (also `free`s the JSON object)
void send_reply_json(struct evhttp_request *request,
                            int code,
                            const char *message,
                            json_t *json) {
    pthread_mutex_lock(&json_mutex);
    struct evbuffer *buf = evhttp_request_get_output_buffer(request);
    char *json_str = json_dumps(json, JSON_COMPACT | JSON_ENSURE_ASCII);
    json_decref(json);
    bool json_str_is_null = false;
    if(json_str == NULL) {
        json_str = "{}";
        json_str_is_null = true;
    }
    evbuffer_add(buf, json_str, strlen(json_str));
    if(!json_str_is_null)
        free(json_str);
    send_reply(request, code, message, buf);
    pthread_mutex_unlock(&json_mutex);
}

// Will wrap an error message in a JSON object before sending it
void send_error(struct evhttp_request *request,
                       int code,
                       const char *message) {
    json_t *error_object = json_object();
    json_object_set_new(error_object, "message", json_string(message));
    send_reply_json(request, code, message, error_object);
}

static void send_error_sp(struct evhttp_request *request,
                          int code,
                          sp_error error) {
    const char *message = sp_error_message(error);
    send_error(request, code, message);
}

struct playlist_handler* register_playlist_callbacks(sp_playlist *playlist,
                                                            struct evhttp_request *request,
                                                            handle_playlist_fn callback,
                                                            sp_playlist_callbacks *playlist_callbacks,
                                                            void *userdata) {
    struct playlist_handler *handler = malloc(sizeof (struct playlist_handler));
    handler->request = request;
    handler->callback = callback;
    handler->playlist_callbacks = playlist_callbacks;
    handler->userdata = userdata;
    sp_playlist_add_callbacks(playlist, handler->playlist_callbacks, handler);
    return handler;
}

static void playlist_dispatch(sp_playlist *playlist, void *userdata) {
    struct playlist_handler *handler = userdata;
    sp_playlist_remove_callbacks(playlist, handler->playlist_callbacks, handler);
    handler->playlist_callbacks = NULL;
    handler->callback(playlist, handler->request, handler->userdata);
    free(handler);
}

static struct playlistcontainer_handler *register_playlistcontainer_callbacks(
                                                                              sp_playlistcontainer *pc,
                                                                              struct evhttp_request *request,
                                                                              handle_playlistcontainer_fn callback,
                                                                              sp_playlistcontainer_callbacks *playlistcontainer_callbacks,
                                                                              void *userdata) {
    struct playlistcontainer_handler *handler = malloc(sizeof (struct playlistcontainer_handler));
    handler->request = request;
    handler->callback = callback;
    handler->playlistcontainer_callbacks = playlistcontainer_callbacks;
    handler->userdata = userdata;
    sp_error error = sp_playlistcontainer_add_callbacks(pc, handler->playlistcontainer_callbacks,
                                                        handler);
    syslog(LOG_DEBUG, "playlistcontainer_add_callbacks: %d", error);
    return handler;
}

static void playlistcontainer_dispatch(sp_playlistcontainer *pc, void *userdata) {
    struct playlistcontainer_handler *handler = userdata;
    sp_playlistcontainer_remove_callbacks(pc,
                                          handler->playlistcontainer_callbacks,
                                          handler);
    handler->playlistcontainer_callbacks = NULL;
    handler->callback(pc, handler->request, handler->userdata);
    free(handler);
}

static void playlist_dispatch_if_loaded(sp_playlist *playlist, void *userdata) {
    if (sp_playlist_is_loaded(playlist))
        playlist_dispatch(playlist, userdata);
}

void playlist_dispatch_if_updated(sp_playlist *playlist,
                                         bool done,
                                         void *userdata) {
    if (done)
        playlist_dispatch(playlist, userdata);
}

// Callbacks for when a playlist is loaded
sp_playlist_callbacks playlist_state_changed_callbacks = {
    .playlist_state_changed = &playlist_dispatch_if_loaded
};

// Callbacks for when a playlist is updated
sp_playlist_callbacks playlist_update_in_progress_callbacks = {
    .playlist_update_in_progress = &playlist_dispatch_if_updated
};

// Callbacks for when subscribers to a playlist is changed
sp_playlist_callbacks playlist_subscribers_changed_callbacks = {
    .subscribers_changed = &playlist_dispatch
};

// Callbacks for when a playlist container is loaded
sp_playlistcontainer_callbacks playlistcontainer_loaded_callbacks = {
    .container_loaded = &playlistcontainer_dispatch
};

// HTTP handlers

// Standard response handler
void not_implemented(sp_playlist *playlist,
                            struct evhttp_request *request,
                            void *userdata) {
    sp_playlist_release(playlist);
    evhttp_send_error(request, HTTP_NOTIMPL, "Not Implemented");
}

// Responds with an entire playlist
void get_playlist(sp_playlist *playlist,
                         struct evhttp_request *request,
                         void *userdata) {
    json_t *json = json_object();
    
    if (playlist_to_json(playlist, json) == NULL) {
        json_decref(json);
        send_error(request, HTTP_ERROR, "");
        return;
    }
    
    sp_playlist_release(playlist);
    send_reply_json(request, HTTP_OK, "OK", json);
}

void get_playlist_tracks(sp_playlist *playlist,
                  struct evhttp_request *request,
                  void *userdata) {
    json_t *json = json_object();
    
    if (playlistsongs_to_json(playlist, json) == NULL) {
        json_decref(json);
        send_error(request, HTTP_ERROR, "");
        return;
    }
    
    sp_playlist_release(playlist);
    send_reply_json(request, HTTP_OK, "OK", json);
    // Release mutex from request
    pthread_mutex_unlock(&playlist_mutex);
}

void get_playlist_collaborative(sp_playlist *playlist,
                                       struct evhttp_request *request,
                                       void *userdata) {
    assert(sp_playlist_is_loaded(playlist));
    json_t *json = json_object();
    playlist_to_json_only_collaborative(playlist, json);
    sp_playlist_release(playlist);
    send_reply_json(request, HTTP_OK, "OK", json);
}

static void get_playlist_subscribers_callback(sp_playlist *playlist,
                                              struct evhttp_request *request,
                                              void *userdata) {
    assert(sp_playlist_is_loaded(playlist));
    sp_subscribers *subscribers = sp_playlist_subscribers(playlist);
    json_t *array = json_array();
    
    for (int i = 0; i < subscribers->count; i++) {
        char *subscriber = subscribers->subscribers[i];
        json_array_append_new(array, json_string(subscriber));
    }
    
    sp_playlist_subscribers_free(subscribers);
    sp_playlist_release(playlist);
    send_reply_json(request, HTTP_OK, "OK", array);
}

void get_playlist_subscribers(sp_playlist *playlist,
                                     struct evhttp_request *request,
                                     void *userdata) {
    assert(sp_playlist_is_loaded(playlist));
    sp_session *session = userdata;
    register_playlist_callbacks(playlist, request,
                                &get_playlist_subscribers_callback,
                                &playlist_subscribers_changed_callbacks,
                                userdata);
    sp_playlist_update_subscribers(session, playlist);
}

// Reads JSON from the requests body. Returns NULL on any error.
static json_t *read_request_body_json(struct evhttp_request *request,
                                      json_error_t *error) {
    struct evbuffer *buf = evhttp_request_get_input_buffer(request);
    size_t buflen = evbuffer_get_length(buf);
    
    if (buflen == 0)
        return NULL;
    
    // Read body
    char *body = malloc(buflen);
    
    if (body == NULL)
        return NULL; // TODO(liesen): Handle memory alloc fail
    
    if (evbuffer_remove(buf, body, buflen) == -1) {
        free(body);
        return NULL;
    }
    
    body[buflen-1] = '\0';
    
    // Parse JSON
    json_t *json = json_loads(body, 0, error);
    free(body);
    return json;
}

static void inbox_post_complete(sp_inbox *inbox, void *userdata) {
    struct evhttp_request *request = userdata;
    sp_error inbox_error = sp_inbox_error(inbox);
    sp_inbox_release(inbox);
    
    switch (inbox_error) {
        case SP_ERROR_OK:
            send_reply(request, HTTP_OK, "OK", NULL);
            break;
            
        case SP_ERROR_NO_SUCH_USER:
            send_error_sp(request, HTTP_NOTFOUND, inbox_error);
            break;
            
        default:
            send_error_sp(request, HTTP_ERROR, inbox_error);
            break;
    }
}

static void get_user_playlists(sp_playlistcontainer *pc,
                               struct evhttp_request *request,
                               void *userdata) {
    json_t *json = json_object();
    json_t *playlists = json_array();
    json_object_set_new(json, "playlists", playlists);
    int status = HTTP_OK;
    
    for (int i = 0; i < sp_playlistcontainer_num_playlists(pc); i++) {
        if (sp_playlistcontainer_playlist_type(pc, i) != SP_PLAYLIST_TYPE_PLAYLIST)
            continue;
        
        sp_playlist *playlist = sp_playlistcontainer_playlist(pc, i);
        
        if (!sp_playlist_is_loaded(playlist)) {
            status = HTTP_PARTIAL;
            continue;
        }
        
        json_t *playlist_json = json_object();
        playlist_to_json(playlist, playlist_json);
        json_array_append_new(playlists, playlist_json);
    }
    
    sp_playlistcontainer_release(pc);
    send_reply_json(request, status, status == HTTP_OK ? "OK" : "Partial Content",
                    json);
}

static void put_user_inbox(const char *user,
                           struct evhttp_request *request,
                           void *userdata) {
    json_error_t loads_error;
    json_t *json = read_request_body_json(request, &loads_error);
    
    if (json == NULL) {
        send_error(request, HTTP_BADREQUEST,
                   loads_error.text ? loads_error.text : "Unable to parse JSON");
        return;
    }
    
    if (!json_is_object(json)) {
        json_decref(json);
        send_error(request, HTTP_BADREQUEST, "Not valid JSON object");
        return;
    }
    
    json_t *tracks_json = json_object_get(json, "tracks");
    
    if (!json_is_array(tracks_json)) {
        json_decref(tracks_json);
        send_error(request, HTTP_BADREQUEST, "tracks is not valid JSON array");
        return;
    }
    
    // Handle empty array
    int num_tracks = (int)json_array_size(tracks_json);
    
    if (num_tracks == 0) {
        send_reply(request, HTTP_OK, "OK", NULL);
        return;
    }
    
    sp_track **tracks = calloc(num_tracks, sizeof (sp_track *));
    int num_valid_tracks = json_to_tracks(tracks_json, tracks, num_tracks);
    
    if (num_valid_tracks == 0) {
        send_error(request, HTTP_BADREQUEST, "No valid tracks");
    } else {
        json_t *message_json = json_object_get(json, "message");
        sp_session *session = userdata;
        sp_inbox *inbox = sp_inbox_post_tracks(session, user, tracks,
                                               num_valid_tracks,
                                               json_is_string(message_json) ? json_string_value(message_json) : "",
                                               &inbox_post_complete, request);
        
        if (inbox == NULL)
            send_error(request, HTTP_ERROR,
                       "Failed to initialize request to add tracks to user's inbox");
    }
    
    json_decref(json);
    free(tracks);
}

void put_playlist(sp_playlist *playlist,
                         struct evhttp_request *request,
                         void *userdata) {
    // TODO(liesen): playlist there so that signatures of all handler methods are
    // the same, but do they have to be?
    assert(playlist == NULL);
    
    sp_session *session = userdata;
    json_error_t loads_error;
    json_t *playlist_json = read_request_body_json(request, &loads_error);
    
    if (playlist_json == NULL) {
        send_error(request, HTTP_BADREQUEST,
                   loads_error.text ? loads_error.text : "Unable to parse JSON");
        return;
    }
    
    // Parse playlist
    if (!json_is_object(playlist_json)) {
        send_error(request, HTTP_BADREQUEST, "Invalid playlist object");
        return;
    }
    
    // Get title
    json_t *title_json = json_object_get(playlist_json, "title");
    
    if (title_json == NULL) {
        json_decref(playlist_json);
        send_error(request, HTTP_BADREQUEST,
                   "Invalid playlist: title is missing");
        return;
    }
    
    if (!json_is_string(title_json)) {
        json_decref(playlist_json);
        send_error(request, HTTP_BADREQUEST,
                   "Invalid playlist: title is not a string");
        return;
    }
    
    char title[kMaxPlaylistTitleLength];
    strncpy(title, json_string_value(title_json), kMaxPlaylistTitleLength);
    json_decref(playlist_json);
    
    // Add new playlist
    sp_playlistcontainer *pc = sp_session_playlistcontainer(session);
    playlist = sp_playlistcontainer_add_new_playlist(pc, title);
    
    if (playlist == NULL) {
        send_error(request, HTTP_ERROR, "Unable to create playlist");
    } else {
        register_playlist_callbacks(playlist, request, &get_playlist,
                                    &playlist_state_changed_callbacks, NULL);
    }
}

void delete_playlist(sp_playlist *playlist,
                            struct evhttp_request *request,
                            void *userdata) {
    if (playlist == NULL) {
        send_error(request, HTTP_ERROR, "Unable to delete playlist");
        return;
    }
    
    struct state *state = userdata;
    sp_session *session = state->session;
    sp_playlistcontainer *pc = sp_session_playlistcontainer(session);
    
    for (int i = 0; i < sp_playlistcontainer_num_playlists(pc); i++) {
        if (sp_playlistcontainer_playlist_type(pc, i) != SP_PLAYLIST_TYPE_PLAYLIST)
            continue;
        
        if (sp_playlistcontainer_playlist(pc, i) != playlist) {
            sp_error remove_error = sp_playlistcontainer_remove_playlist(pc, i);
            
            if (remove_error == SP_ERROR_OK) {
                send_reply(request, HTTP_OK, "OK", NULL);
            } else {
                send_error_sp(request, HTTP_BADREQUEST, remove_error);
            }
            
            return;
        }
    }
    
    send_error(request, HTTP_BADREQUEST, "Unable to delete playlist");
}

void put_playlist_add_tracks(sp_playlist *playlist,
                                    struct evhttp_request *request,
                                    void *userdata) {
    sp_session *session = userdata;
    const char *uri = evhttp_request_get_uri(request);
    struct evkeyvalq query_fields;
    evhttp_parse_query(uri, &query_fields);
    
    // Parse index
    const char *index_field = evhttp_find_header(&query_fields, "index");
    int index;
    
    if (index_field == NULL || sscanf(index_field, "%d", &index) <= 0) {
        index = sp_playlist_num_tracks(playlist);
    }
    
    // Parse JSON
    json_error_t loads_error;
    json_t *json = read_request_body_json(request, &loads_error);
    
    if (json == NULL) {
        sp_playlist_release(playlist);
        send_error(request, HTTP_BADREQUEST,
                   loads_error.text ? loads_error.text : "Unable to parse JSON");
        return;
    }
    
    if (!json_is_array(json)) {
        sp_playlist_release(playlist);
        json_decref(json);
        send_error(request, HTTP_BADREQUEST, "Not valid JSON array");
        return;
    }
    
    // Handle empty array
    int num_tracks = (int)json_array_size(json);
    
    if (num_tracks == 0) {
        sp_playlist_release(playlist);
        send_reply(request, HTTP_OK, "OK", NULL);
        return;
    }
    
    sp_track **tracks = calloc(num_tracks, sizeof (sp_track *));
    int num_valid_tracks = json_to_tracks(json, tracks, num_tracks);
    json_decref(json);
    
    // Bail if no tracks could be read from input
    if (num_valid_tracks == 0) {
        send_error(request, HTTP_BADREQUEST, "No valid tracks");
        free(tracks);
        return;
    }
    
    struct playlist_handler *handler = register_playlist_callbacks(
                                                                   playlist, request, &get_playlist,
                                                                   &playlist_update_in_progress_callbacks, NULL);
    sp_error add_tracks_error = sp_playlist_add_tracks(playlist, tracks,
                                                       num_valid_tracks,
                                                       index, session);
    
    if (add_tracks_error != SP_ERROR_OK) {
        sp_playlist_remove_callbacks(playlist, handler->playlist_callbacks,
                                     handler);
        sp_playlist_release(playlist);
        free(handler);
        send_error_sp(request, HTTP_BADREQUEST, add_tracks_error);
    }
    
    free(tracks);
}

void put_playlist_remove_tracks(sp_playlist *playlist,
                                       struct evhttp_request *request,
                                       void *userdata) {
    // sp_session *session = userdata;
    const char *uri = evhttp_request_get_uri(request);
    struct evkeyvalq query_fields;
    evhttp_parse_query(uri, &query_fields);
    
    // Parse index
    const char *index_field = evhttp_find_header(&query_fields, "index");
    int index;
    
    if (index_field == NULL ||
        sscanf(index_field, "%d", &index) <= 0 ||
        index < 0) {
        sp_playlist_release(playlist);
        send_error(request, HTTP_BADREQUEST,
                   "Bad parameter: index must be numeric");
        return;
    }
    
    const char *count_field = evhttp_find_header(&query_fields, "count");
    int count;
    
    if (count_field == NULL ||
        sscanf(count_field, "%d", &count) <= 0 ||
        count < 1) {
        sp_playlist_release(playlist);
        send_error(request, HTTP_BADREQUEST,
                   "Bad parameter: count must be numeric and positive");
        return;
    }
    
    int *tracks = calloc(count, sizeof(int));
    
    for (int i = 0; i < count; i++)
        tracks[i] = index + i;
    
    struct playlist_handler *handler = register_playlist_callbacks(
                                                                   playlist, request, &get_playlist,
                                                                   &playlist_update_in_progress_callbacks, NULL);
    sp_error remove_tracks_error = sp_playlist_remove_tracks(playlist, tracks,
                                                             count);
    
    if (remove_tracks_error != SP_ERROR_OK) {
        sp_playlist_remove_callbacks(playlist, handler->playlist_callbacks, handler);
        sp_playlist_release(playlist);
        free(handler);
        send_error_sp(request, HTTP_BADREQUEST, remove_tracks_error);
    }
    
    free(tracks);
}

void handle_user_request(struct evhttp_request *request,
                                char *action,
                                const char *canonical_username,
                                sp_session *session) {
    if (action == NULL) {
        evhttp_send_error(request, HTTP_BADREQUEST, "Bad Request");
        return;
    }
    
    int http_method = evhttp_request_get_command(request);
    
    switch (http_method) {
        case EVHTTP_REQ_GET:
            if (strncmp(action, "playlists", 9) == 0) {
                sp_playlistcontainer *pc = sp_session_publishedcontainer_for_user_create(
                                                                                         session, canonical_username);
                
                if (sp_playlistcontainer_is_loaded(pc)) {
                    get_user_playlists(pc, request, session);
                } else {
                    register_playlistcontainer_callbacks(pc, request,
                                                         &get_user_playlists,
                                                         &playlistcontainer_loaded_callbacks,
                                                         session);
                }
                
                return;
            } else if (strncmp(action, "starred", 7) == 0) {
                sp_playlist *playlist = sp_session_starred_for_user_create(session,
                                                                           canonical_username);
                
                if (sp_playlist_is_loaded(playlist)) {
                    get_playlist(playlist, request, session);
                } else {
                    register_playlist_callbacks(playlist, request, &get_playlist,
                                                &playlist_state_changed_callbacks,
                                                session);
                }
                
                return;
            }
            break;
            
        case EVHTTP_REQ_PUT:
        case EVHTTP_REQ_POST:
            if (strncmp(action, "inbox", 5) == 0) {
                put_user_inbox(canonical_username, request, session);
                return;
            }
            break;
    }
    
    evhttp_send_error(request, HTTP_BADREQUEST, "Bad Request");
}
