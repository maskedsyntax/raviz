#ifndef FFT_H
#define FFT_H

#include "../utils/config.h"
#include <stdint.h>

typedef struct FFTContext FFTContext;

// Initialize FFT subsystem
FFTContext* fft_init(const RavizConfig *config);

// Process audio samples and produce smoothed magnitudes.
// 'input_buffer' must be size 'config->fft_size'.
// 'output_bins' must be size 'config->fft_bins'.
void fft_process(FFTContext *ctx, const int16_t *input_buffer, float *output_bins);

void fft_cleanup(FFTContext *ctx);

#endif
