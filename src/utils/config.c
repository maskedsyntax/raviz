#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <toml.h>

#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

void config_init_defaults(RavizConfig *config) {
    config->fps = 30;
    config->audio_rate = 44100;
    config->fft_size = 512; 
    config->fft_bins = 64;
    config->sphere_lat = 40;
    config->sphere_lon = 40;
    config->sphere_scale = 1.0f;
    config->color_mode = COLOR_MODE_NONE; 
    config->intensity = 1.0f;
    config->rotation_speed = 0.05f;
    config->smoothing = 0.15f; 
    config->window_opacity = 1.0f; 
    config->show_fps = false;
    config->audio_device = NULL;
}

static void ensure_config_exists(const char *path) {
    if (access(path, F_OK) != -1) return;

    // Create directories
    char *path_dup = strdup(path);
    char *p = strrchr(path_dup, '/');
    if (p) {
        *p = '\0';
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", path_dup);
        system(cmd);
    }
    free(path_dup);

    FILE *f = fopen(path, "w");
    if (f) {
        fprintf(f, "# Raviz Configuration\n\n");
        fprintf(f, "[render]\n");
        fprintf(f, "fps = 30\n");
        fprintf(f, "sphere_lat = 40\n");
        fprintf(f, "sphere_lon = 40\n");
        fprintf(f, "sphere_scale = 1.0\n");
        fprintf(f, "rotation_speed = 0.05\n");
        fprintf(f, "window_opacity = 1.0\n");
        fprintf(f, "color_mode = \"none\" # none, static, reactive\n\n");
        
        fprintf(f, "[audio]\n");
        fprintf(f, "rate = 44100\n");
        fprintf(f, "fft_size = 512\n");
        fprintf(f, "fft_bins = 64\n");
        fprintf(f, "smoothing = 0.15\n");
        fprintf(f, "intensity = 1.0\n");
        fprintf(f, "# device = \"alsa_output.pci...\"\n");
        fclose(f);
        printf("Created default config at %s\n", path);
    } else {
        fprintf(stderr, "Failed to create config file at %s\n", path);
    }
}

void config_load(RavizConfig *config) {
    const char *home = getenv("HOME");
    if (!home) return;

    char path[512];
    snprintf(path, sizeof(path), "%s/.config/raviz/config.toml", home);

    ensure_config_exists(path);

    FILE *fp = fopen(path, "r");
    if (!fp) return;

    char errbuf[200];
    toml_table_t *conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!conf) {
        fprintf(stderr, "Failed to parse config: %s\n", errbuf);
        return;
    }

    toml_table_t *render = toml_table_in(conf, "render");
    if (render) {
        toml_datum_t fps = toml_int_in(render, "fps");
        if (fps.ok) config->fps = (int)fps.u.i;

        toml_datum_t lat = toml_int_in(render, "sphere_lat");
        if (lat.ok) config->sphere_lat = (int)lat.u.i;

        toml_datum_t lon = toml_int_in(render, "sphere_lon");
        if (lon.ok) config->sphere_lon = (int)lon.u.i;

        toml_datum_t scale = toml_double_in(render, "sphere_scale");
        if (scale.ok) config->sphere_scale = (float)scale.u.d;

        toml_datum_t rot = toml_double_in(render, "rotation_speed");
        if (rot.ok) config->rotation_speed = (float)rot.u.d;
        
        toml_datum_t op = toml_double_in(render, "window_opacity");
        if (op.ok) config->window_opacity = (float)op.u.d;

        toml_datum_t cm = toml_string_in(render, "color_mode");
        if (cm.ok) {
            if (strcmp(cm.u.s, "static") == 0) config->color_mode = COLOR_MODE_STATIC;
            else if (strcmp(cm.u.s, "reactive") == 0) config->color_mode = COLOR_MODE_REACTIVE;
            else config->color_mode = COLOR_MODE_NONE;
            free(cm.u.s);
        }
    }

    toml_table_t *audio = toml_table_in(conf, "audio");
    if (audio) {
        toml_datum_t rate = toml_int_in(audio, "rate");
        if (rate.ok) config->audio_rate = (int)rate.u.i;

        toml_datum_t size = toml_int_in(audio, "fft_size");
        if (size.ok) config->fft_size = (int)size.u.i;
        
        toml_datum_t bins = toml_int_in(audio, "fft_bins");
        if (bins.ok) config->fft_bins = (int)bins.u.i;

        toml_datum_t smooth = toml_double_in(audio, "smoothing");
        if (smooth.ok) config->smoothing = (float)smooth.u.d;

        toml_datum_t inten = toml_double_in(audio, "intensity");
        if (inten.ok) config->intensity = (float)inten.u.d;
        
        toml_datum_t dev = toml_string_in(audio, "device");
        if (dev.ok) {
            config->audio_device = strdup(dev.u.s);
            free(dev.u.s);
        }
    }

    toml_free(conf);
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
        } else if (strcmp(argv[i], "--scale") == 0 && i + 1 < argc) {
            config->sphere_scale = atof(argv[++i]);
        } else if (strcmp(argv[i], "--device") == 0 && i + 1 < argc) {
            config->audio_device = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: raviz [options]\n");
            printf("Options:\n");
            printf("  --fps <int>            Target FPS (default: 30)\n");
            printf("  --bins <int>           Number of frequency bins (default: 64)\n");
            printf("  --lat <int>            Sphere latitude segments (default: 40)\n");
            printf("  --lon <int>            Sphere longitude segments (default: 40)\n");
            printf("  --scale <float>        Sphere scale (default: 1.0)\n");
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