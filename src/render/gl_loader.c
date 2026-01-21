#include "gl_loader.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

PFNGLGENBUFFERS glGenBuffers = NULL;
PFNGLBINDBUFFER glBindBuffer = NULL;
PFNGLBUFFERDATA glBufferData = NULL;
PFNGLDELETEBUFFERS glDeleteBuffers = NULL;

PFNGLCREATESHADER glCreateShader = NULL;
PFNGLSHADERSOURCE glShaderSource = NULL;
PFNGLCOMPILESHADER glCompileShader = NULL;
PFNGLGETSHADERIV glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOG glGetShaderInfoLog = NULL;
PFNGLDELETESHADER glDeleteShader = NULL;

PFNGLCREATEPROGRAM glCreateProgram = NULL;
PFNGLATTACHSHADER glAttachShader = NULL;
PFNGLLINKPROGRAM glLinkProgram = NULL;
PFNGLUSEPROGRAM glUseProgram = NULL;
PFNGLDELETEPROGRAM glDeleteProgram = NULL;

PFNGLGETUNIFORMLOCATION glGetUniformLocation = NULL;
PFNGLUNIFORM1F glUniform1f = NULL;
PFNGLUNIFORM1FV glUniform1fv = NULL;
PFNGLUNIFORMMATRIX4FV glUniformMatrix4fv = NULL;

PFNGLGENVERTEXARRAYS glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAY glBindVertexArray = NULL;
PFNGLDELETEVERTEXARRAYS glDeleteVertexArrays = NULL;

PFNGLENABLEVERTEXATTRIBARRAY glEnableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTER glVertexAttribPointer = NULL;

int load_gl_functions() {
    int errors = 0;
    
    #define LOAD(name) \
        name = (void*)glfwGetProcAddress(#name); \
        if (!name) { \
            fprintf(stderr, "Failed to load GL function: %s\n", #name); \
            errors++; \
        }

    LOAD(glGenBuffers);
    LOAD(glBindBuffer);
    LOAD(glBufferData);
    LOAD(glDeleteBuffers);
    
    LOAD(glCreateShader);
    LOAD(glShaderSource);
    LOAD(glCompileShader);
    LOAD(glGetShaderiv);
    LOAD(glGetShaderInfoLog);
    LOAD(glDeleteShader);
    
    LOAD(glCreateProgram);
    LOAD(glAttachShader);
    LOAD(glLinkProgram);
    LOAD(glUseProgram);
    LOAD(glDeleteProgram);
    
    LOAD(glGetUniformLocation);
    LOAD(glUniform1f);
    LOAD(glUniform1fv);
    LOAD(glUniformMatrix4fv);
    
    LOAD(glGenVertexArrays);
    LOAD(glBindVertexArray);
    LOAD(glDeleteVertexArrays);
    
    LOAD(glEnableVertexAttribArray);
    LOAD(glVertexAttribPointer);
    
    return errors == 0;
}
