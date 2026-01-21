#include "audio.h"
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/def.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct AudioContext {
    pa_simple *s;
    pa_sample_spec ss;
};

AudioContext* audio_init(const RavizConfig *config) {
    AudioContext *ctx = malloc(sizeof(AudioContext));
    if (!ctx) return NULL;

    ctx->ss.format = PA_SAMPLE_S16LE;
    ctx->ss.rate = config->audio_rate;
    ctx->ss.channels = 1; 

    // Request low latency
    pa_buffer_attr ba;
    ba.maxlength = config->fft_size * sizeof(int16_t) * 2; 
    ba.tlength = (uint32_t)-1;
    ba.prebuf = (uint32_t)-1;
    ba.minreq = (uint32_t)-1;
    // Set fragment size to exactly what we want to read per frame
    ba.fragsize = config->fft_size * sizeof(int16_t);

    int error;
    // config->audio_device can be NULL, which is valid for default source
    ctx->s = pa_simple_new(NULL, "Raviz", PA_STREAM_RECORD, config->audio_device, "music", &ctx->ss, NULL, &ba, &error);

    if (!ctx->s) {
        fprintf(stderr, "Error connecting to PulseAudio: %s\n", pa_strerror(error));
        free(ctx);
        return NULL;
    }

    pa_simple_flush(ctx->s, &error);

    return ctx;
}

size_t audio_read(AudioContext *ctx, int16_t *buffer, size_t num_samples) {
    if (!ctx || !ctx->s) return 0;

    int error;
    size_t bytes_to_read = num_samples * sizeof(int16_t);
    
    if (pa_simple_read(ctx->s, buffer, bytes_to_read, &error) < 0) {
        fprintf(stderr, "pa_simple_read() failed: %s\n", pa_strerror(error));
        return 0;
    }

    return num_samples;
}

void audio_cleanup(AudioContext *ctx) {
    if (ctx) {
        if (ctx->s) {
            pa_simple_free(ctx->s);
        }
        free(ctx);
    }
}
