#include "fft.h"
#include <fftw3.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

struct FFTContext {
    int size;
    int num_bins;
    double *in;
    fftw_complex *out;
    fftw_plan plan;
    float *prev_bins; 
    float smoothing_factor;
    float *window;    
    float max_peak;   // Auto-gain control
};

FFTContext* fft_init(const RavizConfig *config) {
    FFTContext *ctx = malloc(sizeof(FFTContext));
    if (!ctx) return NULL;

    ctx->size = config->fft_size;
    ctx->num_bins = config->fft_bins;
    ctx->smoothing_factor = config->smoothing;
    ctx->max_peak = 1.0f; // Initial guess

    ctx->in = (double*)fftw_malloc(sizeof(double) * ctx->size);
    ctx->out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (ctx->size / 2 + 1));
    ctx->prev_bins = calloc(ctx->num_bins, sizeof(float));
    ctx->window = malloc(sizeof(float) * ctx->size);

    ctx->plan = fftw_plan_dft_r2c_1d(ctx->size, ctx->in, ctx->out, FFTW_ESTIMATE);

    for (int i = 0; i < ctx->size; ++i) {
        ctx->window[i] = 0.5 * (1 - cos(2 * M_PI * i / (ctx->size - 1)));
    }

    return ctx;
}

void fft_process(FFTContext *ctx, const int16_t *input_buffer, float *output_bins) {
    double sum_sq = 0.0;
    for (int i = 0; i < ctx->size; ++i) {
        double val = (double)input_buffer[i] / 32768.0;
        sum_sq += val * val;
        ctx->in[i] = val * ctx->window[i];
    }
    double rms = sqrt(sum_sq / ctx->size);

    // STRICT SILENCE GATE
    if (rms < 0.005) {
        memset(output_bins, 0, sizeof(float) * ctx->num_bins);
        memset(ctx->prev_bins, 0, sizeof(float) * ctx->num_bins);
        return; 
    }

    fftw_execute(ctx->plan);

    int spectrum_size = ctx->size / 2 + 1;
    
    // Simple mapping: we have spectrum_size frequencies, want num_bins
    // We skip DC (index 0) usually
    
    int chunk_size = (spectrum_size - 1) / ctx->num_bins;
    if (chunk_size < 1) chunk_size = 1;

    float frame_peak = 0.0001f; 
    float temp_bins[ctx->num_bins];

    for (int i = 0; i < ctx->num_bins; ++i) {
        double mag_sum = 0;
        int start = 1 + i * chunk_size; 
        int end = start + chunk_size;
        if (end > spectrum_size) end = spectrum_size;
        int count = 0;

        for (int j = start; j < end; ++j) {
            double re = ctx->out[j][0];
            double im = ctx->out[j][1];
            mag_sum += sqrt(re * re + im * im);
            count++;
        }
        
        float val = (count > 0) ? (float)(mag_sum / count) : 0.0f;
        temp_bins[i] = val;
        if (val > frame_peak) frame_peak = val;
    }

    // Auto-Gain Control (AGC) Update
    if (frame_peak > ctx->max_peak) {
        ctx->max_peak = frame_peak;
    } else {
        ctx->max_peak = ctx->max_peak * 0.92f + frame_peak * 0.08f;
    }
    if (ctx->max_peak < 0.1f) ctx->max_peak = 0.1f;

    for (int i = 0; i < ctx->num_bins; ++i) {
        if (frame_peak < 0.005f) {
             ctx->prev_bins[i] = 0.0f;
             output_bins[i] = 0.0f;
             continue;
        }

        float normalized_val = temp_bins[i] / ctx->max_peak;
        
        ctx->prev_bins[i] = ctx->prev_bins[i] * ctx->smoothing_factor + normalized_val * (1.0f - ctx->smoothing_factor);
        output_bins[i] = ctx->prev_bins[i];
    }
}

void fft_cleanup(FFTContext *ctx) {
    if (ctx) {
        if (ctx->plan) fftw_destroy_plan(ctx->plan);
        if (ctx->in) fftw_free(ctx->in);
        if (ctx->out) fftw_free(ctx->out);
        free(ctx->prev_bins);
        free(ctx->window);
        free(ctx);
    }
}
