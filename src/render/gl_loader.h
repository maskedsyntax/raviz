#ifndef GL_LOADER_H
#define GL_LOADER_H

#include <GL/gl.h>
#include <stddef.h>

// Define types if not in GL/gl.h (Linux usually has them in gl.h or glext.h, but we can't assume glext.h)
#ifndef GL_MAJOR_VERSION
#define GL_MAJOR_VERSION 0x821B
#endif

// We need ptrdiff_t
#include <stddef.h>

// Typedefs for function pointers
typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

// Constants
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84

// Function Pointers
typedef void (*PFNGLGENBUFFERS)(GLsizei n, GLuint *buffers);
typedef void (*PFNGLBINDBUFFER)(GLenum target, GLuint buffer);
typedef void (*PFNGLBUFFERDATA)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (*PFNGLDELETEBUFFERS)(GLsizei n, const GLuint *buffers);

typedef GLuint (*PFNGLCREATESHADER)(GLenum type);
typedef void (*PFNGLSHADERSOURCE)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (*PFNGLCOMPILESHADER)(GLuint shader);
typedef void (*PFNGLGETSHADERIV)(GLuint shader, GLenum pname, GLint *params);
typedef void (*PFNGLGETSHADERINFOLOG)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (*PFNGLDELETESHADER)(GLuint shader);

typedef GLuint (*PFNGLCREATEPROGRAM)(void);
typedef void (*PFNGLATTACHSHADER)(GLuint program, GLuint shader);
typedef void (*PFNGLLINKPROGRAM)(GLuint program);
typedef void (*PFNGLUSEPROGRAM)(GLuint program);
typedef void (*PFNGLDELETEPROGRAM)(GLuint program);

typedef GLint (*PFNGLGETUNIFORMLOCATION)(GLuint program, const GLchar *name);
typedef void (*PFNGLUNIFORM1F)(GLint location, GLfloat v0);
typedef void (*PFNGLUNIFORM1FV)(GLint location, GLsizei count, const GLfloat *value);
typedef void (*PFNGLUNIFORMMATRIX4FV)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

typedef void (*PFNGLGENVERTEXARRAYS)(GLsizei n, GLuint *arrays);
typedef void (*PFNGLBINDVERTEXARRAY)(GLuint array);
typedef void (*PFNGLDELETEVERTEXARRAYS)(GLsizei n, const GLuint *arrays);

typedef void (*PFNGLENABLEVERTEXATTRIBARRAY)(GLuint index);
typedef void (*PFNGLVERTEXATTRIBPOINTER)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);

// Externs
extern PFNGLGENBUFFERS glGenBuffers;
extern PFNGLBINDBUFFER glBindBuffer;
extern PFNGLBUFFERDATA glBufferData;
extern PFNGLDELETEBUFFERS glDeleteBuffers;

extern PFNGLCREATESHADER glCreateShader;
extern PFNGLSHADERSOURCE glShaderSource;
extern PFNGLCOMPILESHADER glCompileShader;
extern PFNGLGETSHADERIV glGetShaderiv;
extern PFNGLGETSHADERINFOLOG glGetShaderInfoLog;
extern PFNGLDELETESHADER glDeleteShader;

extern PFNGLCREATEPROGRAM glCreateProgram;
extern PFNGLATTACHSHADER glAttachShader;
extern PFNGLLINKPROGRAM glLinkProgram;
extern PFNGLUSEPROGRAM glUseProgram;
extern PFNGLDELETEPROGRAM glDeleteProgram;

extern PFNGLGETUNIFORMLOCATION glGetUniformLocation;
extern PFNGLUNIFORM1F glUniform1f;
extern PFNGLUNIFORM1FV glUniform1fv;
extern PFNGLUNIFORMMATRIX4FV glUniformMatrix4fv;

extern PFNGLGENVERTEXARRAYS glGenVertexArrays;
extern PFNGLBINDVERTEXARRAY glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYS glDeleteVertexArrays;

extern PFNGLENABLEVERTEXATTRIBARRAY glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTER glVertexAttribPointer;

// Load functions
int load_gl_functions();

#endif
