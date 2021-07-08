#include "Typedefs.h"
#include "Clock.h"
#include "Window.h"
#include "OpenGL.h"
#include "Shader.h"
#include "DynamicArray.h"
#include "Simplex.h"
#include "Transform.h"
#include "Vertex.h"
#include "Shader.h"
#include "Camera.h"
#include "Chunk.h"
#include "stb_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <cglm/cglm.h>

static b8 WindowCloseCallback(Window* window, void* userData) {
    return TRUE;
}

static void WindowResizeCallback(Window* window, u32 width, u32 height, void* cameraUserData) {
    Camera* camera = cameraUserData;
    glViewport(0, 0, width, height);
    glm_perspective_resize(cast(f32) width / cast(f32) height, camera->ProjectionMatrix);
}

static b8 WPressed = FALSE;
static b8 APressed = FALSE;
static b8 SPressed = FALSE;
static b8 DPressed = FALSE;
static b8 QPressed = FALSE;
static b8 EPressed = FALSE;
static b8 ShiftPressed = FALSE;
static b8 ChunkLoadingDisabled = FALSE;
static void WindowKeyCallback(Window* window, u32 key, b8 pressed, void* userData) {
    switch (key) {
        case 'W': {
            WPressed = pressed;
        } break;

        case 'A': {
            APressed = pressed;
        } break;

        case 'S': {
            SPressed = pressed;
        } break;

        case 'D': {
            DPressed = pressed;
        } break;

        case 'Q': {
            QPressed = pressed;
        } break;

        case 'E': {
            EPressed = pressed;
        } break;

        case 'R': {
            // NOTE: This requires a compatability context? Should this be used?
            if (pressed) {
                glDisable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        } break;

        case 'F': {
            if (pressed) {
                ChunkLoadingDisabled = !ChunkLoadingDisabled;
            }
        } break;

        case 0x1B: { // TODO: This is escape replace this later its windows specific
            static b8 Locked = TRUE;
            if (pressed) {
                Locked = !Locked;
                if (Locked) {
                    Window_LockCursor(window);
                } else {
                    Window_UnlockCursor(window);
                }
            }
        } break;

        case 0x10: { // TODO: This is shift but also a windows specific keycode!
            ShiftPressed = pressed;
        } break;

        default: {
        } break;
    }
}

static s32 MouseXDelta = 0, MouseYDelta = 0;
static void WindowMouseMoveCallback(Window* window, s32 deltaX, s32 deltaY, void* userData) {
    MouseXDelta += deltaX;
    MouseYDelta += deltaY;
}

int main(int argc, char** argv) {
    Clock_Init();

    Window* window = Window_Create(1280, 720, "Minecraft");
    if (!window) {
        printf("Unable to create window!\n");
        return -1;
    }

    Window_SetCloseCallback(window, WindowCloseCallback, NULL);
    Window_SetKeyCallback(window, WindowKeyCallback, NULL);
    Window_SetMouseMoveCallback(window, WindowMouseMoveCallback, NULL);

    if (!Window_MakeContextCurrent(window)) {
        printf("Unable to initialize OpenGL context!");
        return -1;
    }

    if (!InitializeOpenGLFunctions()) {
        printf("Unable to load OpenGL functions!");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    static const char* VertexShaderSource =
        "#version 440 core\n"
        "\n"
        "layout(location = 0) in vec4 a_Position;\n"
        "layout(location = 1) in vec3 a_Normal;\n"
        "layout(location = 2) in vec2 a_TexCoord;\n"
        "\n"
        "layout(location = 0) out vec3 v_Normal;\n"
        "layout(location = 1) out vec2 v_TexCoord;\n"
        "\n"
        "layout(location = 0) uniform mat4 u_Model;\n"
        "layout(location = 1) uniform mat4 u_View;\n"
        "layout(location = 2) uniform mat4 u_Projection;\n"
        "\n"
        "void main() {\n"
        "   v_Normal = (u_Model * vec4(a_Normal, 0.0)).xyz;\n"
        "   v_TexCoord = a_TexCoord;\n"
        "   gl_Position = u_Projection * u_View * u_Model * a_Position;\n"
        "}\n";

    static const char* FragmentShaderSource =
        "#version 440 core\n"
        "\n"
        "layout(location = 0) out vec4 o_Color;\n"
        "\n"
        "layout(location = 0) in vec3 v_Normal;\n"
        "layout(location = 1) in vec2 v_TexCoord;\n"
        "\n"
        "void main() {\n"
        "   vec3 color = vec3(0.8); // vec3(v_TexCoord, 0.0);\n"
        "   o_Color = vec4(color * max(0.3, (dot(v_Normal, normalize(vec3(0.4, 1.0, -0.3))) + 1.0) * 0.5), 1.0f);\n"
        "}\n";

    GLuint shader = 0;
    CreateShader(VertexShaderSource, FragmentShaderSource, &shader);

    Camera camera = {};
    camera.Transform = (Transform){
        .Position = { 0.0f, 20.0f, 0.0f },
        .Rotation = { 0.0f, 0.0f, 0.0f },
        .Scale = { 1.0f, 1.0f, 1.0f },
    };

    glm_mat4_identity(camera.ProjectionMatrix);
    glm_perspective(70 * cast(f32) (M_PI / 180.0), 0.75f, 0.01f, 1000.0f, camera.ProjectionMatrix);

    Window_SetResizeCallback(window, WindowResizeCallback, &camera);

    Chunk* chunks = DynamicArrayCreate(Chunk);

    Window_Show(window);
    Window_LockCursor(window);

    Clock clock = {};
    Clock_Start(&clock);
    Clock_Update(&clock);
    f64 lastTime = clock.Elapsed;
    while (TRUE) {
        Clock_Update(&clock);
        f32 dt = cast(f32) (clock.Elapsed - lastTime);
        printf("FPS: %f, Chunk Count: %llu\r", 1.0f / dt, DynamicArrayLength(chunks));

        // Camera movement
        {
            camera.Transform.Rotation[1] += cast(f32) MouseXDelta * 0.5f;
            camera.Transform.Rotation[1] = fmodf(camera.Transform.Rotation[1], 360.0f);

            camera.Transform.Rotation[0] += cast(f32) MouseYDelta * 0.5f;

            if (camera.Transform.Rotation[0] > 80.0f) {
                camera.Transform.Rotation[0] = 80.0f;
            }

            if (camera.Transform.Rotation[0] < -80.0f) {
                camera.Transform.Rotation[0] = -80.0f;
            }

            vec3 forward = {};
            forward[0] = sinf(camera.Transform.Rotation[1] * cast(f32) (M_PI / 180.0)) * cosf(camera.Transform.Rotation[0] * cast(f32) (M_PI / 180.0));
            forward[1] = -sinf(camera.Transform.Rotation[0] * cast(f32) (M_PI / 180.0));
            forward[2] = cosf(camera.Transform.Rotation[1] * cast(f32) (M_PI / 180.0)) * cosf(camera.Transform.Rotation[0] * cast(f32) (M_PI / 180.0));
            glm_vec3_normalize(forward);

            vec3 right = {};
            glm_vec3_cross((vec3){ 0.0f, 1.0f, 0.0f }, forward, right);
            glm_vec3_normalize(right);

            vec3 up = {};
            glm_vec3_cross(forward, right, up);
            glm_vec3_normalize(up);

            f32 MoveSpeed = 4.0f * dt;
            if (ShiftPressed) {
                MoveSpeed *= 3.0f;
            }

            if (WPressed) {
                vec3 move = {};
                glm_vec3_mul(forward, (vec3){ MoveSpeed, MoveSpeed, MoveSpeed }, move);
                glm_vec3_add(camera.Transform.Position, move, camera.Transform.Position);
            }
            if (SPressed) {
                vec3 move = {};
                glm_vec3_mul(forward, (vec3){ MoveSpeed, MoveSpeed, MoveSpeed }, move);
                glm_vec3_sub(camera.Transform.Position, move, camera.Transform.Position);
            }
            if (APressed) {
                vec3 move = {};
                glm_vec3_mul(right, (vec3){ MoveSpeed, MoveSpeed, MoveSpeed }, move);
                glm_vec3_sub(camera.Transform.Position, move, camera.Transform.Position);
            }
            if (DPressed) {
                vec3 move = {};
                glm_vec3_mul(right, (vec3){ MoveSpeed, MoveSpeed, MoveSpeed }, move);
                glm_vec3_add(camera.Transform.Position, move, camera.Transform.Position);
            }
            if (EPressed) {
                vec3 move = {};
                glm_vec3_mul(up, (vec3){ MoveSpeed, MoveSpeed, MoveSpeed }, move);
                glm_vec3_add(camera.Transform.Position, move, camera.Transform.Position);
            }
            if (QPressed) {
                vec3 move = {};
                glm_vec3_mul(up, (vec3){ MoveSpeed, MoveSpeed, MoveSpeed }, move);
                glm_vec3_sub(camera.Transform.Position, move, camera.Transform.Position);
            }
        }

        if (!ChunkLoadingDisabled) {
            const u64 chunkSize = 8;
            const s64 chunkRenderDistance = 5;
            const u64 maxModifiedChunksPerFrame = 5;
            u64 chunksModified = 0;
            for (s64 x = -chunkRenderDistance; x <= chunkRenderDistance; x++) {
                for (s64 y = -chunkRenderDistance; y <= chunkRenderDistance; y++) {
                    for (s64 z = -chunkRenderDistance; z <= chunkRenderDistance; z++) {
                        s64 posX = x * chunkSize + cast(s64) ((roundf(camera.Transform.Position[0] / chunkSize) * chunkSize));
                        s64 posY = y * chunkSize + cast(s64) ((roundf(camera.Transform.Position[1] / chunkSize) * chunkSize));
                        s64 posZ = z * chunkSize + cast(s64) ((roundf(camera.Transform.Position[2] / chunkSize) * chunkSize));

                        b8 exists = FALSE;
                        for (u64 i = 0; i < DynamicArrayLength(chunks); i++) {
                            if (posX == chunks[i].Position.x && posY == chunks[i].Position.y && posZ == chunks[i].Position.z) {
                                exists = TRUE;
                                break;
                            }
                        }
                        if (exists) {
                            continue;
                        }

                        Chunk chunk;
                        Chunk_Create(&chunk, posX, posY, posZ, chunkSize, chunkSize, chunkSize, shader);
                        DynamicArrayPush(chunks, chunk);
                        chunksModified++;
                        if (chunksModified > maxModifiedChunksPerFrame) {
                            goto End;
                        }
                    }
                }
            }
            End:

            for (u64 i = 0; i < DynamicArrayLength(chunks); i++) {
                if (_abs64(chunks[i].Position.x - cast(s64) ((roundf(camera.Transform.Position[0] / chunkSize) * chunkSize))) > chunkRenderDistance * cast(s64) chunkSize ||
                    _abs64(chunks[i].Position.y - cast(s64) ((roundf(camera.Transform.Position[1] / chunkSize) * chunkSize))) > chunkRenderDistance * cast(s64) chunkSize ||
                    _abs64(chunks[i].Position.z - cast(s64) ((roundf(camera.Transform.Position[2] / chunkSize) * chunkSize))) > chunkRenderDistance * cast(s64) chunkSize) {
                    Chunk_Destroy(&chunks[i]);
                    DynamicArrayPopAt(chunks, i, NULL);
                    chunksModified++;
                    i--; // TODO: Is this safe?
                }

                if (chunksModified > maxModifiedChunksPerFrame) {
                    break;
                }
            }
        }

        glClearColor(0.4f, 0.6f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        for (u64 i = 0; i < DynamicArrayLength(chunks); i++) {
            Chunk_Draw(&chunks[i], &camera);
        }

        Window_SwapBuffers(window);

        lastTime = clock.Elapsed;

        MouseXDelta = 0;
        MouseYDelta = 0;

        if (!Window_PollEvents(window)) {
            break;
        }
    }

    Window_Hide(window);

    for (u64 i = 0; i < DynamicArrayLength(chunks); i++) {
        Chunk_Destroy(&chunks[i]);
    }
    glDeleteProgram(shader);

    Window_Destroy(window);
	return 0;
}
