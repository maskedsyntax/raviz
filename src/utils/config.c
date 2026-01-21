#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void config_init_defaults(RavizConfig *config) {
    config->fps = 30;
    config->audio_rate = 44100;
    config->fft_size = 512; 
    config->fft_bins = 64;
    config->sphere_lat = 40;
    config->sphere_lon = 40;
    config->color_mode = COLOR_MODE_NONE; 
    config->intensity = 1.0f;
    config->rotation_speed = 0.05f;
    config->smoothing = 0.15f; 
    config->window_opacity = 1.0f; 
    config->show_fps = false;
    config->audio_device = NULL;
}

int config_parse_args(RavizConfig *config, int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--fps") == 0 && i + 1 < argc) {
            config->fps = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--bins") == 0 && i + 1 < argc) {
            config->fft_bins = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--lat") == 0 && i + 1 < argc) {
            config->sphere_lat = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--lon") == 0 && i + 1 < argc) {
            config->sphere_lon = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--color") == 0 && i + 1 < argc) {
            char *mode = argv[++i];
            if (strcmp(mode, "static") == 0) config->color_mode = COLOR_MODE_STATIC;
            else if (strcmp(mode, "reactive") == 0) config->color_mode = COLOR_MODE_REACTIVE;
            else config->color_mode = COLOR_MODE_NONE;
        } else if (strcmp(argv[i], "--no-color") == 0) {
            config->color_mode = COLOR_MODE_NONE;
        } else if (strcmp(argv[i], "--intensity") == 0 && i + 1 < argc) {
            config->intensity = atof(argv[++i]);
        } else if (strcmp(argv[i], "--opacity") == 0 && i + 1 < argc) {
            config->window_opacity = atof(argv[++i]);
        } else if (strcmp(argv[i], "--rotation-speed") == 0 && i + 1 < argc) {
            config->rotation_speed = atof(argv[++i]);
        } else if (strcmp(argv[i], "--device") == 0 && i + 1 < argc) {
            config->audio_device = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: raviz [options]\n");
            printf("Options:\n");
            printf("  --fps <int>            Target FPS (default: 30)\n");
            printf("  --bins <int>           Number of frequency bins (default: 64)\n");
            printf("  --lat <int>            Sphere latitude segments (default: 40)\n");
            printf("  --lon <int>            Sphere longitude segments (default: 40)\n");
            printf("  --color <mode>         static|reactive|none (default: none)\n");
            printf("  --no-color             Disable color\n");
            printf("  --intensity <float>    Reaction intensity (default: 1.0)\n");
            printf("  --opacity <float>      Window opacity 0.0-1.0 (default: 1.0)\n");
            printf("  --rotation-speed <float> Speed (default: 0.05)\n");
            printf("  --device <name>        PulseAudio source device name\n");
            return 1; 
        }
    }
    return 0;
}
