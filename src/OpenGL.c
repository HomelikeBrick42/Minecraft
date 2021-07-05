#include "OpenGL.h"

#define GL_FUNCTION(name, ret, ...) PFN_ ## name name = NULL;
GL_FUNCTIONS
#undef GL_FUNCTION

#include <stdio.h>
#include <Windows.h>

static void* GetGLFunc(const char *name) {
    void* func = wglGetProcAddress(name);
    if (func == cast(void*) 0 ||
        func == cast(void*) 1 ||
        func == cast(void*) 2 ||
        func == cast(void*) 3 ||
        func == cast(void*) -1) {
        HMODULE module = LoadLibraryA("opengl32.dll");
        func = GetProcAddress(module, name);
    }
    return func;
}

b8 InitializeOpenGLFunctions() {
    #define GL_FUNCTION(name, ret, ...) name = GetGLFunc(#name); if (!name) { printf("Unable to load OpenGL function: '" #name "'\n"); return FALSE; }
    GL_FUNCTIONS
    #undef GL_FUNCTION
    return TRUE;
}
