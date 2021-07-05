#pragma once

#include "Typedefs.h"

typedef struct Window Window;
typedef b8 (*Window_CloseCallback)(Window* window, void* userData);
typedef void (*Window_ResizeCallback)(Window* window, u32 width, u32 height, void* userData);
typedef void (*Window_KeyCallback)(Window* window, u32 key, b8 pressed, void* userData);
typedef void (*Window_MouseMoveCallback)(Window* window, s32 deltaX, s32 deltaY, void* userData);

Window* Window_Create(u32 width, u32 height, const char* title);
void Window_Destroy(Window* window);

b8 Window_MakeContextCurrent(Window* window);

void Window_Show(Window* window);
void Window_Hide(Window* window);

void Window_LockCursor(Window* window);
void Window_UnlockCursor(Window* window);

void Window_SetCloseCallback(Window* window, Window_CloseCallback callback, void* userData);
void Window_SetResizeCallback(Window* window, Window_ResizeCallback callback, void* userData);
void Window_SetKeyCallback(Window* window, Window_KeyCallback callback, void* userData);
void Window_SetMouseMoveCallback(Window* window, Window_MouseMoveCallback callback, void* userData);

b8 Window_PollEvents(Window* window);
void Window_SwapBuffers(Window* window);
