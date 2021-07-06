#include "Chunk.h"
#include "DynamicArray.h"
#include "Simplex.h"

#include <memory.h>
#include <stdlib.h>

static u16 GetBlock(vec3 position) {
    const f32 Scale2D = 0.02f;
    const f32 Scale3D = 0.1f;
    f32 groundHeight = snoise2(position[0] * 0.002f, position[2] * 0.002f) * 15.0f;

    if (position[1] > groundHeight) {
        const f32 frequency = 10.0f;
        f32 noise = (snoise2(position[0] * Scale2D, position[2] * Scale2D) + 1.0f) * 0.5f;
        noise *= frequency;
        noise -= snoise3(position[0] * Scale3D, position[1] * Scale3D, position[2] * Scale3D);
        return noise > position[1] - groundHeight ? BlockID_Stone : BlockID_Air;
    } else {
        f32 noise = snoise3(position[0] * Scale3D, position[1] * Scale3D, position[2] * Scale3D);
        return noise < 0.0f ? BlockID_Stone : BlockID_Air;
    }
}

void Chunk_Create(Chunk* chunk, vec3 center, u32 width, u32 height, u32 depth, GLuint shader) {
    *chunk = (Chunk){
        .Center = { center[0], center[1], center[2] },
        .Width = width,
        .Height = height,
        .Depth = depth,
        .Blocks = DynamicArrayCreate_(width * height * depth, sizeof(u16)),
        .Vertices = DynamicArrayCreate(Vertex),
        .Indices = DynamicArrayCreate(u32),
        .Shader = shader,
    };

    glGenVertexArrays(1, &chunk->VertexArray);
    glBindVertexArray(chunk->VertexArray);

    for (u32 x = 0; x < width; x++) {
        for (u32 y = 0; y < height; y++) {
            for (u32 z = 0; z < depth; z++) {
                u32 index = x + (y * width) + (z * width * height);
                vec3 position = {
                    chunk->Center[0] + cast(f32) x - (cast(f32) width * 0.5f),
                    chunk->Center[1] + cast(f32) y - (cast(f32) height * 0.5f),
                    chunk->Center[2] + cast(f32) z - (cast(f32) depth * 0.5f),
                };
                chunk->Blocks[index] = GetBlock(position);
            }
        }
    }

    Chunk_RecalculateMesh(chunk);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, cast(const void*) 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, cast(const void*) sizeof(vec3));
}

void Chunk_Destroy(Chunk* chunk) {
    DynamicArrayDestroy(chunk->Blocks);
    DynamicArrayDestroy(chunk->Vertices);
    DynamicArrayDestroy(chunk->Indices);
    glDeleteBuffers(1, &chunk->VertexBuffer);
    glDeleteBuffers(1, &chunk->IndexBuffer);
    glDeleteVertexArrays(1, &chunk->VertexArray);
}

void Chunk_Draw(Chunk* chunk, Camera* camera) {
    mat4 modelMatrix;
    glm_mat4_identity(modelMatrix);

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

void Chunk_RecalculateMesh(Chunk* chunk) {
    DynamicArrayLength(chunk->Vertices) = 0;
    DynamicArrayLength(chunk->Indices) = 0;

    u32 currentIndex = 0;
    for (u32 x = 0; x < chunk->Width; x++) {
        for (u32 y = 0; y < chunk->Height; y++) {
            for (u32 z = 0; z < chunk->Depth; z++) {
                u32 index = x + (y * chunk->Width) + (z * chunk->Width * chunk->Height);
                vec3 position = {
                    chunk->Center[0] + cast(f32) x - (cast(f32) chunk->Width * 0.5f),
                    chunk->Center[1] + cast(f32) y - (cast(f32) chunk->Height * 0.5f),
                    chunk->Center[2] + cast(f32) z - (cast(f32) chunk->Depth * 0.5f),
                };

                if (chunk->Blocks[index] == BlockID_Air) {
                    continue;
                }

                // Top
                if (GetBlock((vec3){ position[0], position[1] + 1, position[2] }) == BlockID_Air) {
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

                // Bottom
                if (GetBlock((vec3){ position[0], position[1] - 1, position[2] }) == BlockID_Air) {
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                            -0.5f + position[0],
                            -0.5f + position[1],
                             0.5f + position[2],
                        },
                        .Normal = {
                             0.0f,
                            -1.0f,
                             0.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                             0.5f + position[0],
                            -0.5f + position[1],
                             0.5f + position[2],
                        },
                        .Normal = {
                             0.0f,
                            -1.0f,
                             0.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                             0.5f + position[0],
                            -0.5f + position[1],
                            -0.5f + position[2],
                        },
                        .Normal = {
                             0.0f,
                            -1.0f,
                             0.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                            -0.5f + position[0],
                            -0.5f + position[1],
                            -0.5f + position[2],
                        },
                        .Normal = {
                             0.0f,
                            -1.0f,
                             0.0f,
                        },
                    }));

                    DynamicArrayPush(chunk->Indices, currentIndex + 2);
                    DynamicArrayPush(chunk->Indices, currentIndex + 1);
                    DynamicArrayPush(chunk->Indices, currentIndex + 0);

                    DynamicArrayPush(chunk->Indices, currentIndex + 3);
                    DynamicArrayPush(chunk->Indices, currentIndex + 2);
                    DynamicArrayPush(chunk->Indices, currentIndex + 0);

                    currentIndex += 4;
                }

                // Left
                if (GetBlock((vec3){ position[0] - 1, position[1], position[2] }) == BlockID_Air) {
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                            -0.5f + position[0],
                             0.5f + position[1],
                            -0.5f + position[2],
                        },
                        .Normal = {
                            -1.0f,
                             0.0f,
                             0.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                            -0.5f + position[0],
                             0.5f + position[1],
                             0.5f + position[2],
                        },
                        .Normal = {
                            -1.0f,
                             0.0f,
                             0.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                            -0.5f + position[0],
                            -0.5f + position[1],
                             0.5f + position[2],
                        },
                        .Normal = {
                            -1.0f,
                             0.0f,
                             0.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                            -0.5f + position[0],
                            -0.5f + position[1],
                            -0.5f + position[2],
                        },
                        .Normal = {
                            -1.0f,
                             0.0f,
                             0.0f,
                        },
                    }));

                    DynamicArrayPush(chunk->Indices, currentIndex + 2);
                    DynamicArrayPush(chunk->Indices, currentIndex + 1);
                    DynamicArrayPush(chunk->Indices, currentIndex + 0);

                    DynamicArrayPush(chunk->Indices, currentIndex + 3);
                    DynamicArrayPush(chunk->Indices, currentIndex + 2);
                    DynamicArrayPush(chunk->Indices, currentIndex + 0);

                    currentIndex += 4;
                }

                // Right
                if (GetBlock((vec3){ position[0] + 1, position[1], position[2] }) == BlockID_Air) {
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                             0.5f + position[0],
                             0.5f + position[1],
                            -0.5f + position[2],
                        },
                        .Normal = {
                             1.0f,
                             0.0f,
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
                             1.0f,
                             0.0f,
                             0.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                             0.5f + position[0],
                            -0.5f + position[1],
                             0.5f + position[2],
                        },
                        .Normal = {
                             1.0f,
                             0.0f,
                             0.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                             0.5f + position[0],
                            -0.5f + position[1],
                            -0.5f + position[2],
                        },
                        .Normal = {
                             1.0f,
                             0.0f,
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

                // Front
                if (GetBlock((vec3){ position[0], position[1], position[2] + 1 }) == BlockID_Air) {
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                            -0.5f + position[0],
                             0.5f + position[1],
                             0.5f + position[2],
                        },
                        .Normal = {
                             0.0f,
                             0.0f,
                             1.0f,
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
                             0.0f,
                             1.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                             0.5f + position[0],
                            -0.5f + position[1],
                             0.5f + position[2],
                        },
                        .Normal = {
                             0.0f,
                             0.0f,
                             1.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                            -0.5f + position[0],
                            -0.5f + position[1],
                             0.5f + position[2],
                        },
                        .Normal = {
                             0.0f,
                             0.0f,
                             1.0f,
                        },
                    }));

                    DynamicArrayPush(chunk->Indices, currentIndex + 2);
                    DynamicArrayPush(chunk->Indices, currentIndex + 1);
                    DynamicArrayPush(chunk->Indices, currentIndex + 0);

                    DynamicArrayPush(chunk->Indices, currentIndex + 3);
                    DynamicArrayPush(chunk->Indices, currentIndex + 2);
                    DynamicArrayPush(chunk->Indices, currentIndex + 0);

                    currentIndex += 4;
                }

                // Back
                if (GetBlock((vec3){ position[0], position[1], position[2] - 1 }) == BlockID_Air) {
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                            -0.5f + position[0],
                             0.5f + position[1],
                            -0.5f + position[2],
                        },
                        .Normal = {
                             0.0f,
                             0.0f,
                            -1.0f,
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
                             0.0f,
                            -1.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                             0.5f + position[0],
                            -0.5f + position[1],
                            -0.5f + position[2],
                        },
                        .Normal = {
                             0.0f,
                             0.0f,
                            -1.0f,
                        },
                    }));
                    DynamicArrayPush(chunk->Vertices, ((Vertex){
                        .Position = {
                            -0.5f + position[0],
                            -0.5f + position[1],
                            -0.5f + position[2],
                        },
                        .Normal = {
                             0.0f,
                             0.0f,
                            -1.0f,
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

    glGenBuffers(1, &chunk->VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, DynamicArraySize(chunk->Vertices), chunk->Vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &chunk->IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, DynamicArraySize(chunk->Indices), chunk->Indices, GL_STATIC_DRAW);
}
