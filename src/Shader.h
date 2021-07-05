#pragma once

#include "Typedefs.h"
#include "OpenGL.h"

b8 CreateShader(const char* vertexSource, const char* fragmentSource, GLuint* outShader);
