#pragma once

#include "Typedefs.h"
#include "Transform.h"
#include "Vertex.h"
#include "OpenGL.h"
#include "Camera.h"

typedef enum BlockID {
    BlockID_Air   = 0,
    BlockID_Stone = 1,
} BlockID;

typedef struct Chunk {
    vec3 Center;
    u32 Width;
    u32 Height;
    u32 Depth;
    u16* Blocks;
    Vertex* Vertices;
    u32* Indices;
    GLuint VertexArray;
    GLuint VertexBuffer;
    GLuint IndexBuffer;
    GLuint Shader;
} Chunk;

void Chunk_Create(Chunk* chunk, vec3 center, u32 width, u32 height, u32 depth, GLuint shader);
void Chunk_Destroy(Chunk* chunk);

void Chunk_Draw(Chunk* chunk, Camera* camera);
void Chunk_RecalculateMesh(Chunk* chunk);
