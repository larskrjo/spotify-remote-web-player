#include <libspotify/api.h>
#include <apr.h>
#include <stdlib.h>
#include "globals.h"


struct track_tokens_t {
  sp_track **tracks;
  int num_tracks;
  int index;
};

struct output_baton_t {
  sp_session *session;
  sp_playlist *playlist;
  sp_track **tracks;
  int num_tracks;
};
