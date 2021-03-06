//
//  audio.h
//  webserver
//
//  Created by Lars Kristian Johansen on 03/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#ifndef webserver_audio_h
#define webserver_audio_h

#include <pthread.h>
#include <stdint.h>
#include "queue.h"

typedef struct audio_fifo_data {
	TAILQ_ENTRY(audio_fifo_data) link;
	int channels;
	int rate;
	int nsamples;
	int16_t samples[0];
} audio_fifo_data_t;

typedef struct audio_fifo {
	TAILQ_HEAD(, audio_fifo_data) q;
	int qlen;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} audio_fifo_t;

extern void audio_init(audio_fifo_t *af);
extern void audio_fifo_flush(audio_fifo_t *af);
audio_fifo_data_t* audio_get(audio_fifo_t *af);

#endif
