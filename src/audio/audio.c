#include "audio.h"
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/def.h>
#include <pulse/introspect.h>
#include <pulse/context.h>
#include <pulse/mainloop.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct AudioContext {
    pa_simple *s;
    pa_sample_spec ss;
};

// --- Auto-detection Logic ---

typedef struct {
    char *default_sink_name;
    char *monitor_source_name;
    int done;
} PaAutoDetectData;

static void get_server_info_cb(pa_context *c, const pa_server_info *i, void *userdata) {
    PaAutoDetectData *data = (PaAutoDetectData*)userdata;
    if (i && i->default_sink_name) {
        data->default_sink_name = strdup(i->default_sink_name);
    }
    // Don't mark as done yet, we need to find the sink to get its monitor
    if (!data->default_sink_name) data->done = 1; // Fail fast
}

static void get_sink_info_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    PaAutoDetectData *data = (PaAutoDetectData*)userdata;
    if (eol < 0) {
        data->done = 1;
        return;
    }
    if (!i) return;

    if (data->default_sink_name && strcmp(i->name, data->default_sink_name) == 0) {
        if (i->monitor_source_name) {
            data->monitor_source_name = strdup(i->monitor_source_name);
        }
        data->done = 1;
    }
}

static void context_state_cb(pa_context *c, void *userdata) {
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_READY:
        case PA_CONTEXT_TERMINATED:
        case PA_CONTEXT_FAILED:
            // Handled in loop
            break;
        default:
            break;
    }
}

char* get_default_monitor_source() {
    pa_mainloop *m = pa_mainloop_new();
    pa_mainloop_api *api = pa_mainloop_get_api(m);
    pa_context *c = pa_context_new(api, "RavizFinder");
    
    pa_context_set_state_callback(c, context_state_cb, NULL);
    pa_context_connect(c, NULL, 0, NULL);

    PaAutoDetectData data = {0};
    int state = 0; // 0: connecting, 1: get_server, 2: get_sink, 3: done

    // Run loop
    for(;;) {
        pa_mainloop_iterate(m, 1, NULL);
        
        if (pa_context_get_state(c) == PA_CONTEXT_READY) {
            if (state == 0) {
                pa_context_get_server_info(c, get_server_info_cb, &data);
                state = 1;
            } else if (state == 1 && data.default_sink_name) {
                pa_context_get_sink_info_list(c, get_sink_info_cb, &data);
                state = 2;
            } else if (state == 2 && data.done) {
                break;
            } else if (state == 1 && data.done && !data.default_sink_name) {
                break; // Failed to get server info
            }
        } else if (pa_context_get_state(c) == PA_CONTEXT_FAILED || 
                   pa_context_get_state(c) == PA_CONTEXT_TERMINATED) {
            break;
        }
    }

    if (data.default_sink_name) free(data.default_sink_name);
    
    pa_context_disconnect(c);
    pa_context_unref(c);
    pa_mainloop_free(m);

    return data.monitor_source_name;
}

// --- Main Audio Init ---

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
    ba.fragsize = config->fft_size * sizeof(int16_t);

    int error;
    char *device_name = config->audio_device;
    char *auto_device = NULL;

    // If no device specified, try to find the monitor of the default sink
    if (!device_name) {
        printf("[Audio] Detecting default monitor source...\n");
        auto_device = get_default_monitor_source();
        if (auto_device) {
            printf("[Audio] Found monitor: %s\n", auto_device);
            device_name = auto_device;
        } else {
            printf("[Audio] Could not auto-detect monitor. Using default input (mic?).\n");
        }
    } else {
        printf("[Audio] Using configured device: %s\n", device_name);
    }

    ctx->s = pa_simple_new(NULL, "Raviz", PA_STREAM_RECORD, device_name, "Visualization", &ctx->ss, NULL, &ba, &error);

    if (auto_device) free(auto_device);

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
