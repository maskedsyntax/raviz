#ifndef RENDER_H
#define RENDER_H

#include "../utils/config.h"

typedef struct RenderContext RenderContext;

// Initialize the renderer (Window, OpenGL, Shaders)
RenderContext* render_init(const RavizConfig *config);

// Update render state based on audio data
void render_update(RenderContext *ctx, const float *fft_bins, float dt);

// Handle resize (called by GLFW callback usually, or manually)
void render_resize(RenderContext *ctx, int width, int height);

// Draw the frame
void render_draw(RenderContext *ctx);

// Check if window should close
int render_should_close(RenderContext *ctx);

// Cleanup
void render_cleanup(RenderContext *ctx);

#endif