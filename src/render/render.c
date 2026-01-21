#include "render.h"
#include "gl_loader.h"
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_FFT_BINS 64

struct RenderContext {
    GLFWwindow *window;
    RavizConfig config;
    
    GLuint shader_program;
    GLuint vao, vbo, ebo;
    int num_indices;
    
    GLint u_time;
    GLint u_fft_data;
    GLint u_resolution;
    GLint u_view;
    GLint u_projection;
    GLint u_model;
    GLint u_intensity;
    GLint u_color_mode;
    
    float time;
    float fft_data[MAX_FFT_BINS];
};

const char *vs_source = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"
    "\n"
    "uniform float time;\n"
    "uniform float fft_data[64];\n"
    "uniform float intensity;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "\n"
    "out vec3 vNormal;\n"
    "out vec3 vPos;\n"
    "out float vDisplacement;\n"
    "\n"
    "// Simple pseudo-random noise\n"
    "float random(vec3 st) {\n"
    "    return fract(sin(dot(st.xyz, vec3(12.9898,78.233,45.5432))) * 43758.5453123);\n"
    "}\n"
    "\n"
    "void main() {\n"
    "    vec3 pos = aPos;\n"
    "    \n"
    "    float low_energy = 0.0;\n"
    "    for(int i=0; i<5; i++) low_energy += fft_data[i];\n"
    "    low_energy /= 5.0;\n"
    "    \n"
    "    float mid_energy = 0.0;\n"
    "    for(int i=5; i<20; i++) mid_energy += fft_data[i];\n"
    "    mid_energy /= 15.0;\n"
    "    \n"
    "    float displacement = 0.0;\n"
    "    \n"
    "    displacement += low_energy * 0.2 * intensity;\n"
    "    \n"
    "    float noise = random(pos + time * 0.1);\n"
    "    displacement += noise * mid_energy * 0.5 * intensity;\n"
    "    \n"
    "    vec3 new_pos = pos + aNormal * displacement;\n"
    "    \n"
    "    vDisplacement = displacement;\n"
    "    vNormal = aNormal;\n"
    "    vPos = new_pos;\n"
    "    \n"
    "    gl_Position = projection * view * model * vec4(new_pos, 1.0);\n"
    "}\n";

const char *fs_source = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "\n"
    "in vec3 vNormal;\n"
    "in vec3 vPos;\n"
    "in float vDisplacement;\n"
    "\n"
    "uniform float time;\n"
    "uniform int color_mode;\n"
    "\n"
    "void main() {\n"
    "    vec3 norm = normalize(vNormal);\n"
    "    vec3 viewDir = normalize(-vPos); \n"
    "    \n"
    "    // Fresnel glow\n"
    "    float fresnel = pow(1.0 - max(dot(norm, vec3(0.0, 0.0, 1.0)), 0.0), 3.0);\n"
    "    \n"
    "    vec3 color = vec3(0.1, 0.4, 0.8); \n"
    "    \n"
    "    color += vec3(0.8, 0.2, 0.5) * vDisplacement * 5.0;\n"
    "    \n"
    "    color += vec3(0.5, 0.8, 1.0) * fresnel * 0.8;\n"
    "    \n"
    "    FragColor = vec4(color, 1.0);\n"
    "}\n";

GLuint compile_shader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Shader compilation failed: %s\n", infoLog);
        return 0;
    }
    return shader;
}

void generate_sphere(int lat_segments, int lon_segments, GLuint *vao, GLuint *vbo, GLuint *ebo, int *num_indices) {
    int num_vertices = (lat_segments + 1) * (lon_segments + 1);
    *num_indices = lat_segments * lon_segments * 6;
    
    float *vertices = malloc(num_vertices * 6 * sizeof(float)); 
    unsigned int *indices = malloc(*num_indices * sizeof(unsigned int));
    
    int v_idx = 0;
    for (int y = 0; y <= lat_segments; ++y) {
        for (int x = 0; x <= lon_segments; ++x) {
            float x_segment = (float)x / (float)lon_segments;
            float y_segment = (float)y / (float)lat_segments;
            float x_pos = cos(x_segment * 2.0f * GLM_PI) * sin(y_segment * GLM_PI);
            float y_pos = cos(y_segment * GLM_PI);
            float z_pos = sin(x_segment * 2.0f * GLM_PI) * sin(y_segment * GLM_PI);
            
            vertices[v_idx++] = x_pos;
            vertices[v_idx++] = y_pos;
            vertices[v_idx++] = z_pos;
            
            vertices[v_idx++] = x_pos;
            vertices[v_idx++] = y_pos;
            vertices[v_idx++] = z_pos;
        }
    }
    
    int i_idx = 0;
    for (int y = 0; y < lat_segments; ++y) {
        for (int x = 0; x < lon_segments; ++x) {
            indices[i_idx++] = (y + 1) * (lon_segments + 1) + x;
            indices[i_idx++] = y * (lon_segments + 1) + x;
            indices[i_idx++] = y * (lon_segments + 1) + x + 1;
            
            indices[i_idx++] = (y + 1) * (lon_segments + 1) + x;
            indices[i_idx++] = y * (lon_segments + 1) + x + 1;
            indices[i_idx++] = (y + 1) * (lon_segments + 1) + x + 1;
        }
    }
    
    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    glGenBuffers(1, ebo);
    
    glBindVertexArray(*vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * 6 * sizeof(float), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, *num_indices * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    free(vertices);
    free(indices);
}

void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error: %s\n", description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

RenderContext* render_init(const RavizConfig *config) {
    RenderContext *ctx = malloc(sizeof(RenderContext));
    if (!ctx) return NULL;
    
    ctx->config = *config;
    ctx->time = 0.0f;
    for(int i=0; i<MAX_FFT_BINS; i++) ctx->fft_data[i] = 0.0f;
    
    glfwSetErrorCallback(error_callback);
    
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        free(ctx);
        return NULL;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    
    ctx->window = glfwCreateWindow(800, 800, "Raviz", NULL, NULL);
    if (!ctx->window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        free(ctx);
        return NULL;
    }
    
    glfwMakeContextCurrent(ctx->window);
    glfwSetFramebufferSizeCallback(ctx->window, framebuffer_size_callback);
    
    if (!load_gl_functions()) {
        fprintf(stderr, "Failed to load OpenGL functions\n");
        glfwDestroyWindow(ctx->window);
        glfwTerminate();
        free(ctx);
        return NULL;
    }
    
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_source);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_source);
    
    ctx->shader_program = glCreateProgram();
    glAttachShader(ctx->shader_program, vs);
    glAttachShader(ctx->shader_program, fs);
    glLinkProgram(ctx->shader_program);
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    ctx->u_time = glGetUniformLocation(ctx->shader_program, "time");
    ctx->u_fft_data = glGetUniformLocation(ctx->shader_program, "fft_data");
    ctx->u_model = glGetUniformLocation(ctx->shader_program, "model");
    ctx->u_view = glGetUniformLocation(ctx->shader_program, "view");
    ctx->u_projection = glGetUniformLocation(ctx->shader_program, "projection");
    ctx->u_intensity = glGetUniformLocation(ctx->shader_program, "intensity");
    
    generate_sphere(config->sphere_lat, config->sphere_lon, &ctx->vao, &ctx->vbo, &ctx->ebo, &ctx->num_indices);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return ctx;
}

void render_update(RenderContext *ctx, const float *fft_bins, float dt) {
    if (!ctx) return; 
    
    ctx->time += dt;
    
    int bins = ctx->config.fft_bins;
    if (bins > MAX_FFT_BINS) bins = MAX_FFT_BINS;
    
    for (int i=0; i<bins; i++) {
        ctx->fft_data[i] = fft_bins[i];
    }
}

void render_draw(RenderContext *ctx) {
    if (!ctx || !ctx->window) return;
    
    glClearColor(0.0f, 0.0f, 0.0f, ctx->config.window_opacity); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(ctx->shader_program);    
    glUniform1f(ctx->u_time, ctx->time);
    glUniform1fv(ctx->u_fft_data, MAX_FFT_BINS, ctx->fft_data);
    glUniform1f(ctx->u_intensity, ctx->config.intensity);
    
    int width, height;
    glfwGetFramebufferSize(ctx->window, &width, &height);
    float aspect = (float)width / (float)height;
    
    mat4 model, view, projection;
    glm_mat4_identity(model);
    glm_mat4_identity(view);
    glm_mat4_identity(projection);
    
    glm_rotate(model, ctx->time * ctx->config.rotation_speed, (vec3){0.0f, 1.0f, 0.0f});
    glm_translate(view, (vec3){0.0f, 0.0f, -3.0f});
    glm_perspective(glm_rad(45.0f), aspect, 0.1f, 100.0f, projection);
    
    glUniformMatrix4fv(ctx->u_model, 1, GL_FALSE, (float*)model);
    glUniformMatrix4fv(ctx->u_view, 1, GL_FALSE, (float*)view);
    glUniformMatrix4fv(ctx->u_projection, 1, GL_FALSE, (float*)projection);
    
    glBindVertexArray(ctx->vao);
    glDrawElements(GL_TRIANGLES, ctx->num_indices, GL_UNSIGNED_INT, 0);
    
    glfwSwapBuffers(ctx->window);
    glfwPollEvents();
}

int render_should_close(RenderContext *ctx) {
    if (!ctx || !ctx->window) return 1;
    return glfwWindowShouldClose(ctx->window);
}

void render_resize(RenderContext *ctx, int width, int height) {
    if (ctx && ctx->window) {
        glfwSetWindowSize(ctx->window, width, height);
    }
}

void render_cleanup(RenderContext *ctx) {
    if (ctx) {
        glDeleteVertexArrays(1, &ctx->vao);
        glDeleteBuffers(1, &ctx->vbo);
        glDeleteBuffers(1, &ctx->ebo);
        glDeleteProgram(ctx->shader_program);
        
        if (ctx->window) {
            glfwDestroyWindow(ctx->window);
        }
        glfwTerminate();
        free(ctx);
    }
}
