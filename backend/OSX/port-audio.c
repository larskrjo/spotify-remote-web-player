//
//  port-audio.c
//  webserver
//
//  Created by Lars Kristian Johansen on 10/04/14.
//  Copyright (c) 2014 Lars Kristian Johansen. All rights reserved.
//

#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include "helper.h"
#include <string.h>
#include "portaudio.h"

 /* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
//#define SAMPLE_RATE  (44100)
#define SAMPLE_RATE (44100)
//#define FRAMES_PER_BUFFER (1024)
#define FRAMES_PER_BUFFER (2048)
#define NUM_CHANNELS    (2)
/* #define DITHER_FLAG     (paDitherOff)  */
#define DITHER_FLAG     (0)
/* @todo Underflow and overflow is disabled until we fix priming of blocking write. */
#define CHECK_OVERFLOW  (0)
#define CHECK_UNDERFLOW  (0)


/* Select sample format. */
#define PA_SAMPLE_TYPE  paInt16
#define SAMPLE_SIZE (2)
#define SAMPLE_SILENCE  (0)
#define CLEAR(a) memset( (a), 0,  FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE )
#define PRINTF_S_FORMAT "%d"


static void* audio_start(void *aux)
{
    audio_fifo_t *af = (audio_fifo_t*)aux;
    audio_fifo_data_t *afd;
    
    /* -- initialize PortAudio -- */
    PaStreamParameters inputParameters, outputParameters;
    PaStream *stream = NULL;
    PaError err;
    char *sampleBlock;
    int i;
    int numBytes;
    
    
    printf("patest_read_write_wire.c\n");
    fflush(stdout);
    
    numBytes = FRAMES_PER_BUFFER * NUM_CHANNELS * SAMPLE_SIZE ;
    sampleBlock = (char *) malloc( numBytes );
    if( sampleBlock == NULL )
    {
        printf("Could not allocate record array.\n");
        exit(1);
    }
    CLEAR( sampleBlock );
    err = Pa_Initialize();
    if( err != paNoError ) goto error;
    
    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    printf( "Input device # %d.\n", inputParameters.device );
    printf( "Input LL: %g s\n", Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency );
    printf( "Input HL: %g s\n", Pa_GetDeviceInfo( inputParameters.device )->defaultHighInputLatency );
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultHighInputLatency ;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    
    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    printf( "Output device # %d.\n", outputParameters.device );
    printf( "Output LL: %g s\n", Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency );
    printf( "Output HL: %g s\n", Pa_GetDeviceInfo( outputParameters.device )->defaultHighOutputLatency );
    outputParameters.channelCount = NUM_CHANNELS;
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultHighOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    /* -- setup -- */
    
    err = Pa_OpenStream(
                        &stream,
                        &inputParameters,
                        &outputParameters,
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paClipOff, /* we won't output out of range samples so don't bother clipping them */
                        NULL, /* no callback, use blocking API */
                        NULL); /* no callback, so no callback userData */
    if( err != paNoError ) goto error;
    
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;
    fflush(stdout);
    for( i=0; i>=0; ++i ) {
        afd = audio_get(af);
        Pa_WriteStream(stream, afd->samples, afd->nsamples);
        free(afd);
    }
    err = Pa_StopStream( stream );
    if( err != paNoError ) goto error;
    
    CLEAR( sampleBlock );
    free( sampleBlock );
    
    Pa_Terminate();
    return NULL; // OK
    
    xrun:
    if( stream ) {
        Pa_AbortStream( stream );
        Pa_CloseStream( stream );
    }
    free( sampleBlock );
    Pa_Terminate();
    if( err & paInputOverflow )
        fprintf( stderr, "Input Overflow.\n" );
    if( err & paOutputUnderflow )
        fprintf( stderr, "Output Underflow.\n" );
        return NULL; // NOT OK
    
    error:
        if( stream ) {
            Pa_AbortStream( stream );
            Pa_CloseStream( stream );
        }
    free( sampleBlock );
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return NULL; // NOT OK
}

void audio_init(audio_fifo_t *af)
{
//    sp_session_preferred_bitrate(g_sess, SP_BITRATE_320k);
    pthread_t tid;
    
    TAILQ_INIT(&af->q);
    af->qlen = 0;
    
    pthread_mutex_init(&af->mutex, NULL);
    pthread_cond_init(&af->cond, NULL);
    
    pthread_create(&tid, NULL, audio_start, af);
}
