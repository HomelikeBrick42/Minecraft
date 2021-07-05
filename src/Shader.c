#include "Shader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

b8 CreateShader(const char* vertexSource, const char* fragmentSource, GLuint* outShader) {
    *outShader = 0;

    GLuint vertexShader = 0;
    {
        GLint vertexSourceLength = strlen(vertexSource);
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, &vertexSourceLength);
        glCompileShader(vertexShader);

        GLint compiled = FALSE;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint maxLength = 0;
            glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

            GLchar* infoLog = malloc(maxLength);

            GLsizei length = 0;
            glGetShaderInfoLog(vertexShader, maxLength, &length, infoLog);

            printf("Vertex Shader Compilation Failed: %.*s\n", length, infoLog);
            
            free(infoLog);

            return FALSE;
        }
    }

    GLuint fragmentShader = 0;
    {
        GLint fragmentSourceLength = strlen(fragmentSource);

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, &fragmentSourceLength);
        glCompileShader(fragmentShader);

        GLint compiled = FALSE;
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint maxLength = 0;
            glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

            GLchar* infoLog = malloc(maxLength);

            GLsizei length = 0;
            glGetShaderInfoLog(fragmentShader, maxLength, &length, infoLog);

            printf("Fragment Shader Compilation Failed: %.*s\n", length, infoLog);

            free(infoLog);

            return FALSE;
        }
    }

    GLuint shader = glCreateProgram();
    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glLinkProgram(shader);

    GLint linked = FALSE;
    glGetProgramiv(shader, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint maxLength = 0;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        GLchar* infoLog = malloc(maxLength);

        GLsizei length = 0;
        glGetProgramInfoLog(shader, maxLength, &length, infoLog);

        printf("Shader Linking Failed: %.*s\n", length, infoLog);

        free(infoLog);

        return FALSE;
    }

    glDetachShader(shader, vertexShader);
    glDeleteShader(vertexShader);

    glDetachShader(shader, fragmentShader);
    glDeleteShader(fragmentShader);

    *outShader = shader;
    return TRUE;
}
