#define _POSIX_C_SOURCE 199309L
#include "utils/config.h"
#include "audio/audio.h"
#include "fft/fft.h"
#include "render/render.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

// Shared state between Audio Thread and Render Thread (Main)
typedef struct {
    float *fft_output;
    int fft_bins;
    pthread_mutex_t mutex;
    volatile int running;
    
    // Audio/FFT contexts managed by the thread
    RavizConfig config;
} AudioThreadState;

static volatile int keep_running = 1;

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        keep_running = 0;
    }
}

long get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000L + ts.tv_nsec;
}

void* audio_thread_func(void *arg) {
    AudioThreadState *state = (AudioThreadState*)arg;
    
    AudioContext *audio = audio_init(&state->config);
    if (!audio) {
        fprintf(stderr, "[Audio] Failed to init audio. Thread exiting.\n");
        return NULL;
    }
    
    FFTContext *fft = fft_init(&state->config);
    if (!fft) {
        fprintf(stderr, "[Audio] Failed to init FFT. Thread exiting.\n");
        audio_cleanup(audio);
        return NULL;
    }
    
    int16_t *audio_buffer = malloc(state->config.fft_size * sizeof(int16_t));
    float *local_fft_output = malloc(state->config.fft_bins * sizeof(float));
    
    while (state->running) {
        size_t read = audio_read(audio, audio_buffer, state->config.fft_size);
        if (read < state->config.fft_size) {
            struct timespec req = {0, 1000000}; // 1ms
            nanosleep(&req, NULL);
            continue;
        }
        
        fft_process(fft, audio_buffer, local_fft_output);
        
        pthread_mutex_lock(&state->mutex);
        memcpy(state->fft_output, local_fft_output, state->config.fft_bins * sizeof(float));
        pthread_mutex_unlock(&state->mutex);
    }
    
    free(audio_buffer);
    free(local_fft_output);
    fft_cleanup(fft);
    audio_cleanup(audio);
    return NULL;
}

int main(int argc, char **argv) {
    RavizConfig config;
    config_init_defaults(&config);
    if (config_parse_args(&config, argc, argv)) {
        return 0; 
    }

    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    RenderContext *render = render_init(&config);
    if (!render) {
        fprintf(stderr, "Failed to initialize Renderer.\n");
        return 1;
    }

    AudioThreadState audio_state;
    audio_state.config = config;
    audio_state.fft_bins = config.fft_bins;
    audio_state.fft_output = calloc(config.fft_bins, sizeof(float));
    audio_state.running = 1;
    pthread_mutex_init(&audio_state.mutex, NULL);

    pthread_t audio_thread;
    if (pthread_create(&audio_thread, NULL, audio_thread_func, &audio_state) != 0) {
        fprintf(stderr, "Failed to create audio thread.\n");
        render_cleanup(render);
        return 1;
    }

    printf("Raviz started. Press Ctrl+C or close window to exit.\n");
    if (config.audio_device) {
        printf("Listening on device: %s\n", config.audio_device);
    } else {
        printf("Listening on default audio device.\n");
    }

    long frame_duration_ns = 1000000000L / config.fps;
    long last_time = get_time_ns();
    
    float *render_fft_buffer = malloc(config.fft_bins * sizeof(float));

    while (keep_running && !render_should_close(render)) {
        long current_time = get_time_ns();
        long elapsed = current_time - last_time;
        float dt = (float)elapsed / 1000000000.0f;
        last_time = current_time;

        pthread_mutex_lock(&audio_state.mutex);
        memcpy(render_fft_buffer, audio_state.fft_output, config.fft_bins * sizeof(float));
        pthread_mutex_unlock(&audio_state.mutex);

        render_update(render, render_fft_buffer, dt);
        render_draw(render);

        long work_time = get_time_ns() - current_time;
        long sleep_ns = frame_duration_ns - work_time;
        if (sleep_ns > 0) {
            struct timespec req = {0, sleep_ns};
            nanosleep(&req, NULL);
        }
    }

    audio_state.running = 0;
    pthread_join(audio_thread, NULL);
    pthread_mutex_destroy(&audio_state.mutex);

    free(render_fft_buffer);
    free(audio_state.fft_output);
    render_cleanup(render);
    
    printf("Raviz stopped.\n");

    return 0;
}
