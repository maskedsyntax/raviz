#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fftw3.h"
#include "pulse/simple.h"
#include "pulse/error.h"

// --- FFTW Stubs ---

void *fftw_malloc(size_t n) {
    return malloc(n);
}

void fftw_free(void *p) {
    free(p);
}

struct fftw_plan_s {
    int n;
    double *in;
    fftw_complex *out;
};

fftw_plan fftw_plan_dft_r2c_1d(int n, double *in, fftw_complex *out, unsigned flags) {
    fftw_plan p = (fftw_plan)malloc(sizeof(struct fftw_plan_s));
    p->n = n;
    p->in = in;
    p->out = out;
    return p;
}

void fftw_execute(const fftw_plan p) {
    // Simple mock: just copy input to output magnitude roughly
    // Or just leave it as is.
    // Let's do a fake FFT: fill output with some values based on input
    for (int i = 0; i < p->n / 2 + 1; ++i) {
        // Real part
        p->out[i][0] = p->in[i] * 0.5; 
        // Imag part
        p->out[i][1] = 0.0;
    }
}

void fftw_destroy_plan(fftw_plan p) {
    free(p);
}

// --- PulseAudio Stubs ---

struct pa_simple {
    int dummy;
};

pa_simple* pa_simple_new(const char *server, const char *name, pa_stream_direction_t dir,
    const char *dev, const char *stream_name, const pa_sample_spec *ss,
    const void *map, const void *attr, int *error) {
    
    static int warned = 0;
    if (!warned) {
        fprintf(stderr, "[STUB] pa_simple_new: Created mock audio stream.\n");
        warned = 1;
    }
    return (pa_simple*)malloc(sizeof(pa_simple));
}

void pa_simple_free(pa_simple *s) {
    free(s);
}

int pa_simple_read(pa_simple *s, void *data, size_t bytes, int *error) {
    // Fill with random noise for visualization
    int16_t *ptr = (int16_t*)data;
    size_t samples = bytes / sizeof(int16_t);
    for (size_t i = 0; i < samples; ++i) {
        ptr[i] = (rand() % 20000) - 10000;
    }
    // Simulate some delay to match audio rate (approx)
    // 44100Hz, say we read 1024 samples. That's ~23ms.
    // usleep(23000); // Need unistd.h, but keep it simple.
    return 0;
}

const char* pa_strerror(int error) {
    return "Stub error";
}

