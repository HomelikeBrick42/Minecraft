#pragma once

#include "Typedefs.h"

typedef char GLchar;
typedef b8 GLboolean;
typedef s8 GLbyte;
typedef u8 GLubyte;
typedef s16 GLshort;
typedef u16 GLushort;
typedef s32 GLint;
typedef u32 GLuint;
typedef s32 GLfixed;
typedef s64 GLint64;
typedef u64 GLuint64;
typedef u32 GLsizei;
typedef u32 GLenum;
typedef s64 GLintptr;
typedef u64 GLsizeiptr;
typedef u64 GLsync;
typedef u32 GLbitfield;
typedef f32 GLfloat;
typedef f32 GLclampf;
typedef f64 GLdouble;
typedef f64 GLclampd;

#define GL_TRUE 1
#define GL_FALSE 0

#define GL_COLOR_BUFFER_BIT 16384
#define GL_STENCIL_BUFFER_BIT 1024
#define GL_DEPTH_BUFFER_BIT 256

#define GL_DEPTH_TEST 2929
#define GL_BLEND 3042
#define GL_CULL_FACE 2884

#define GL_SRC_ALPHA 770
#define GL_ONE_MINUS_SRC_ALPHA 771

#define GL_FRONT 1028
#define GL_BACK 1029
#define GL_FRONT_AND_BACK 1032

#define GL_LINE 6913
#define GL_FILL 6914

#define GL_TRIANGLES 4

#define GL_FRAGMENT_SHADER 35632
#define GL_VERTEX_SHADER 35633

#define GL_COMPILE_STATUS 35713
#define GL_LINK_STATUS 35714
#define GL_INFO_LOG_LENGTH 35716

#define GL_FLOAT 5126
#define GL_UNSIGNED_INT 5125

#define GL_ARRAY_BUFFER 34962
#define GL_ELEMENT_ARRAY_BUFFER 34963

#define GL_STATIC_DRAW 35044
#define GL_DYNAMIC_DRAW 35048

#define GL_FUNCTIONS \
    GL_FUNCTION(glClear, void, GLbitfield mask) \
    GL_FUNCTION(glClearColor, void, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) \
    \
    GL_FUNCTION(glEnable, void, GLenum cap) \
    GL_FUNCTION(glDisable, void, GLenum cap) \
    GL_FUNCTION(glBlendFunc, void, GLenum sfactor, GLenum dfactor) \
    GL_FUNCTION(glCullFace, void, GLenum mode) \
    \
    GL_FUNCTION(glPolygonMode, void, GLenum face, GLenum mode) \
    \
    GL_FUNCTION(glDrawElements, void, GLenum mode, GLsizei count, GLenum type, const void* indices) \
    \
    GL_FUNCTION(glViewport, void, GLint x, GLint y, GLsizei width, GLsizei height) \
    \
    GL_FUNCTION(glCreateShader, GLuint, GLenum shaderType) \
    GL_FUNCTION(glShaderSource, void, GLuint shader, GLsizei count, const GLchar** string, const GLint* length) \
    GL_FUNCTION(glCompileShader, void, GLuint shader) \
    GL_FUNCTION(glGetShaderiv, void, GLuint shader, GLenum pname, GLint* params) \
    GL_FUNCTION(glGetShaderInfoLog, void, GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog) \
    GL_FUNCTION(glDeleteShader, void, GLuint shader) \
    \
    GL_FUNCTION(glCreateProgram, GLuint, void) \
    GL_FUNCTION(glAttachShader, void, GLuint program, GLuint shader) \
    GL_FUNCTION(glLinkProgram, void, GLuint program) \
    GL_FUNCTION(glGetProgramiv, void, GLuint program, GLenum pname, GLint* params) \
    GL_FUNCTION(glGetProgramInfoLog, void, GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog) \
    GL_FUNCTION(glDetachShader, void, GLuint program, GLuint shader) \
    GL_FUNCTION(glUseProgram, void, GLuint program) \
    GL_FUNCTION(glDeleteProgram, void, GLuint program) \
    \
    GL_FUNCTION(glUniformMatrix4fv, void, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) \
    \
    GL_FUNCTION(glGenVertexArrays, void, GLsizei n, GLuint* arrays) \
    GL_FUNCTION(glBindVertexArray, void, GLuint array) \
    GL_FUNCTION(glVertexAttribPointer, void, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) \
    GL_FUNCTION(glEnableVertexAttribArray, void, GLuint index) \
    GL_FUNCTION(glDeleteVertexArrays, void, GLsizei n, const GLuint* arrays) \
    \
    GL_FUNCTION(glGenBuffers, void, GLsizei n, GLuint* buffers) \
    GL_FUNCTION(glBindBuffer, void, GLenum target, GLuint buffer) \
    GL_FUNCTION(glBufferData, void, GLenum target, GLsizeiptr size, const void* data, GLenum usage) \
    GL_FUNCTION(glDeleteBuffers, void, GLsizei n, const GLuint* buffers)

#define GL_FUNCTION(name, ret, ...) typedef ret (_cdecl *PFN_ ## name)(__VA_ARGS__);
GL_FUNCTIONS
#undef GL_FUNCTION

#define GL_FUNCTION(name, ret, ...) extern PFN_ ## name name;
GL_FUNCTIONS
#undef GL_FUNCTION

b8 InitializeOpenGLFunctions();
