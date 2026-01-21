#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    COLOR_MODE_NONE,
    COLOR_MODE_STATIC,
    COLOR_MODE_REACTIVE
} ColorMode;

typedef struct {
    int fps;
    int audio_rate;
    int fft_size;       // Size of FFT buffer (e.g. 1024)
    int fft_bins;       // Number of output bins (e.g. 64)
    int sphere_lat;     // Latitude segments
    int sphere_lon;     // Longitude segments
    ColorMode color_mode;
    float intensity;    // Global scaling for reaction
    float rotation_speed;
    float smoothing;    // 0.0 to 1.0
    float window_opacity; // 0.0 (transparent) to 1.0 (solid black)
    bool show_fps;
    char *audio_device; // PulseAudio source name or NULL for default
} RavizConfig;

// Initialize with defaults
void config_init_defaults(RavizConfig *config);

// Parse CLI args (simple version)
int config_parse_args(RavizConfig *config, int argc, char **argv);

#endif
