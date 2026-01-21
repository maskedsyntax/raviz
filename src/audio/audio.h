#ifndef AUDIO_H
#define AUDIO_H

#include "../utils/config.h"
#include <stdint.h>
#include <stddef.h>

typedef struct AudioContext AudioContext;

// Initialize audio subsystem
AudioContext* audio_init(const RavizConfig *config);

// Read raw samples into buffer. Returns number of samples read.
// 'buffer' must be large enough to hold 'num_samples' * sizeof(int16_t).
size_t audio_read(AudioContext *ctx, int16_t *buffer, size_t num_samples);

// Cleanup
void audio_cleanup(AudioContext *ctx);

#endif
