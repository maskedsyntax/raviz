#ifndef PULSE_SIMPLE_H
#define PULSE_SIMPLE_H

#include <stdint.h>
#include <stddef.h>

typedef struct pa_simple pa_simple;

typedef enum pa_sample_format {
    PA_SAMPLE_U8,
    PA_SAMPLE_ALAW,
    PA_SAMPLE_ULAW,
    PA_SAMPLE_S16LE,
    PA_SAMPLE_S16BE,
    PA_SAMPLE_FLOAT32LE,
    PA_SAMPLE_FLOAT32BE,
    PA_SAMPLE_S32LE,
    PA_SAMPLE_S32BE,
    PA_SAMPLE_S24LE,
    PA_SAMPLE_S24BE,
    PA_SAMPLE_S24_32LE,
    PA_SAMPLE_S24_32BE,
    PA_SAMPLE_MAX,
    PA_SAMPLE_INVALID = -1
} pa_sample_format_t;

typedef struct pa_sample_spec {
    pa_sample_format_t format;
    uint32_t rate;
    uint8_t channels;
} pa_sample_spec;

typedef enum pa_stream_direction {
    PA_STREAM_NODIRECTION,
    PA_STREAM_PLAYBACK,
    PA_STREAM_RECORD,
    PA_STREAM_UPLOAD,
    PA_STREAM_BLOCK_PLAYBACK,
    PA_STREAM_BLOCK_RECORD
} pa_stream_direction_t;

pa_simple* pa_simple_new(const char *server, const char *name, pa_stream_direction_t dir,
    const char *dev, const char *stream_name, const pa_sample_spec *ss,
    const void *map, const void *attr, int *error);

void pa_simple_free(pa_simple *s);

int pa_simple_read(pa_simple *s, void *data, size_t bytes, int *error);

const char* pa_strerror(int error);

#endif
