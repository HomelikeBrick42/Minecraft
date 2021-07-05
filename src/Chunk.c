#include "Chunk.h"
#include "DynamicArray.h"
#include "Simplex.h"

#include <memory.h>

void Chunk_Create(Chunk* chunk, vec3 center, u32 width, u32 height, u32 depth, GLuint shader) {
    *chunk = (Chunk){
        .Center = { center[0], center[1], center[2] },
        .Width = width,
        .Height = height,
        .Depth = depth,
        .Blocks = DynamicArrayCreate(u16),
        .Vertices = DynamicArrayCreate(Vertex),
        .Indices = DynamicArrayCreate(u32),
        .Shader = shader,
    };

    glGenVertexArrays(1, &chunk->VertexArray);
    glBindVertexArray(chunk->VertexArray);

    for (u32 x = 0; x < width; x++) {
        for (u32 y = 0; y < height; y++) {
            for (u32 z = 0; z < depth; z++) {
                DynamicArrayPush(chunk->Blocks, BlockID_Stone);
            }
        }
    }

    u32 currentIndex = 0;
    for (u32 x = 0; x < width; x++) {
        for (u32 y = 0; y < height; y++) {
            for (u32 z = 0; z < depth; z++) {
                u32 index = x + (y * width) + (z * width * height);
                vec3 position = {
                    cast(f32) x - (cast(f32) width * 0.5f + 0.5f),
                    cast(f32) y - (cast(f32) height * 0.5f + 0.5f),
                    cast(f32) z - (cast(f32) depth * 0.5f + 0.5f),
                };

                if (chunk->Blocks[index] == BlockID_Air) {
                    continue;
                }

                // Top
                {
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                            -0.5f + position[0],
                             0.5f + position[1],
                             0.5f + position[2],
                        },
                        .Normal = {
                            0.0f,
                            1.0f,
                            0.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                             0.5f + position[0],
                             0.5f + position[1],
                             0.5f + position[2],
                        },
                        .Normal = {
                            0.0f,
                            1.0f,
                            0.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                             0.5f + position[0],
                             0.5f + position[1],
                            -0.5f + position[2],
                        },
                        .Normal = {
                            0.0f,
                            1.0f,
                            0.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                            -0.5f + position[0],
                             0.5f + position[1],
                            -0.5f + position[2],
                        },
                        .Normal = {
                            0.0f,
                            1.0f,
                            0.0f,
                        },
                    }));

                    DynamicArrayPush(chunk->Indices, currentIndex + 0);
                    DynamicArrayPush(chunk->Indices, currentIndex + 1);
                    DynamicArrayPush(chunk->Indices, currentIndex + 2);

                    DynamicArrayPush(chunk->Indices, currentIndex + 0);
                    DynamicArrayPush(chunk->Indices, currentIndex + 2);
                    DynamicArrayPush(chunk->Indices, currentIndex + 3);

                    currentIndex += 4;
                }
            }
        }
    }

    // DynamicArrayPush(chunk->Vertices, ((Vertex){ .Position = { -0.5f,  0.5f, 0.0f }, .Normal = { 0.0f, 0.0f, -1.0f } }));
    // DynamicArrayPush(chunk->Vertices, ((Vertex){ .Position = {  0.5f,  0.5f, 0.0f }, .Normal = { 0.0f, 0.0f, -1.0f } }));
    // DynamicArrayPush(chunk->Vertices, ((Vertex){ .Position = {  0.5f, -0.5f, 0.0f }, .Normal = { 0.0f, 0.0f, -1.0f } }));
    // DynamicArrayPush(chunk->Vertices, ((Vertex){ .Position = { -0.5f, -0.5f, 0.0f }, .Normal = { 0.0f, 0.0f, -1.0f } }));

    // DynamicArrayPush(chunk->Indices, 0);
    // DynamicArrayPush(chunk->Indices, 1);
    // DynamicArrayPush(chunk->Indices, 3);

    // DynamicArrayPush(chunk->Indices, 1);
    // DynamicArrayPush(chunk->Indices, 2);
    // DynamicArrayPush(chunk->Indices, 3);

    glGenBuffers(1, &chunk->VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, DynamicArraySize(chunk->Vertices), chunk->Vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &chunk->IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, DynamicArraySize(chunk->Indices), chunk->Indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, cast(const void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, cast(const void*) sizeof(vec3));
}

void Chunk_Destroy(Chunk* chunk) {
    DynamicArrayDestroy(chunk->Vertices);
    DynamicArrayDestroy(chunk->Indices);
    glDeleteBuffers(1, &chunk->VertexBuffer);
    glDeleteBuffers(1, &chunk->IndexBuffer);
    glDeleteVertexArrays(1, &chunk->VertexArray);
}

void Chunk_Draw(Chunk* chunk, Camera* camera) {
    mat4 modelMatrix;
    glm_mat4_identity(modelMatrix);
    glm_translate(modelMatrix, chunk->Center);

    glUseProgram(chunk->Shader);

    glUniformMatrix4fv(0, 1, GL_FALSE, cast(GLfloat*) modelMatrix);

    mat4 viewMatrix;
    Transform_ToMatrix(&camera->Transform, viewMatrix);
    glm_mat4_inv(viewMatrix, viewMatrix);
    glUniformMatrix4fv(1, 1, GL_FALSE, cast(GLfloat*) viewMatrix);

    glUniformMatrix4fv(2, 1, GL_FALSE, cast(GLfloat*) camera->ProjectionMatrix);

    glBindVertexArray(chunk->VertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->VertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IndexBuffer);
    glDrawElements(GL_TRIANGLES, DynamicArrayLength(chunk->Indices), GL_UNSIGNED_INT, NULL);
}
