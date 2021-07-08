#include "Window.h"

#include <stdio.h>
#include <stdlib.h>

#include <Windows.h>

typedef struct Window {
    HINSTANCE Instance;
    HWND Handle;
    HDC DeviceContext;
    HGLRC GLContext;
    b8 LockCursor;
    RECT OldClip;
    Window_CloseCallback CloseCallback;
    void* CloseCallbackUserData;
    Window_ResizeCallback ResizeCallback;
    void* ResizeCallbackUserData;
    Window_KeyCallback KeyCallback;
    void* KeyCallbackUserData;
    Window_MouseMoveCallback MouseMoveCallback;
    void* MouseMoveCallbackUserData;
} Window;

static const char* WindowClassName = "WindowClass"; // TODO:
static u64 WindowCount = 0;

static const DWORD WindowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
static const DWORD WindowStyleEx = WS_EX_APPWINDOW;

static LRESULT CALLBACK WindowMessageCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    LRESULT result = 0;

    Window* window = cast(Window*) GetWindowLongPtrA(hWnd, GWLP_USERDATA);

    if (window) {
        switch (message) {
            case WM_QUIT:
            case WM_CLOSE: {
                if (!window->CloseCallback || (window->CloseCallback && window->CloseCallback(window, window->CloseCallbackUserData))) {
                    PostQuitMessage(0);
                }
            } break;

            case WM_SIZE: {
                if (window->ResizeCallback) {
                    RECT clientRect = {};
                    GetClientRect(window->Handle, &clientRect);
                    s32 width = clientRect.right - clientRect.left;
                    s32 height = clientRect.bottom - clientRect.top;
                    if (width > 0 && height > 0) {
                        window->ResizeCallback(window, cast(u32) width, cast(u32) height, window->ResizeCallbackUserData);
                    }
                }

                if (window->LockCursor) {
                    RECT windowRect = {};
                    GetWindowRect(window->Handle, &windowRect);
                    RECT clipRect = {};
                    AdjustWindowRectEx(&clipRect, WindowStyle, 0, WindowStyleEx);
                    windowRect.left -= clipRect.left + 10;
                    windowRect.right -= clipRect.right + 10;
                    windowRect.top -= clipRect.top + 10;
                    windowRect.bottom -= clipRect.bottom + 10;
                    ClipCursor(&windowRect);
                }
            } break;

            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYUP: {
                if (window->KeyCallback) {
                    b8 pressed = message == WM_SYSKEYDOWN || message == WM_KEYDOWN;
                    window->KeyCallback(window, cast(u32) wParam, pressed, window->KeyCallbackUserData);
                }

                result = DefWindowProcA(window->Handle, message, wParam, lParam);
            } break;

            case WM_INPUT: {
                UINT dwSize;
                GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
                BYTE lpb[dwSize];

                if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
                    OutputDebugStringA("GetRawInputData does not return correct size !\n");
                }

                RAWINPUT* raw = (RAWINPUT*)lpb;

                if (raw->header.dwType == RIM_TYPEMOUSE) {
                    if (window->KeyCallback) {
                        u32 xPos = LOWORD(lParam);
                        u32 yPos = HIWORD(lParam);
                        window->MouseMoveCallback(window, raw->data.mouse.lLastX, raw->data.mouse.lLastY, window->MouseMoveCallbackUserData);
                    }
                }
            } break;

            default: {
                result = DefWindowProcA(window->Handle, message, wParam, lParam);
            } break;
        }
    } else {
        result = DefWindowProcA(hWnd, message, wParam, lParam);
    }

    return result;
}

Window* Window_Create(u32 width, u32 height, const char* title) {
    HINSTANCE instance = GetModuleHandle(NULL);

    if (WindowCount == 0) {
        WNDCLASSEXA windowClass = {
            .cbSize = sizeof(windowClass),
            .style = CS_OWNDC,
            .lpfnWndProc = WindowMessageCallback,
            .hInstance = instance,
            .hCursor = LoadCursor(NULL, IDC_ARROW),
            .lpszClassName = WindowClassName,
        };

        if (!RegisterClassExA(&windowClass)) {
            return NULL;
        }
    }
    WindowCount++;

    Window* window = malloc(sizeof(Window));
    memset(window, 0, sizeof(Window));
    window->Instance = instance;

    RECT windowRect = {};
    windowRect.left = 100;
    windowRect.right = windowRect.left + width;
    windowRect.top = 100;
    windowRect.bottom = windowRect.top + height;
    AdjustWindowRectEx(&windowRect, WindowStyle, 0, WindowStyleEx);

    u32 windowWidth = windowRect.right - windowRect.left;
    u32 windowHeight = windowRect.bottom - windowRect.top;

    window->Handle = CreateWindowExA(
        WindowStyleEx,
        WindowClassName,
        title,
        WindowStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowWidth, windowHeight,
        NULL,
        NULL,
        window->Instance,
        NULL
    );

    if (!window->Handle) {
        Window_Destroy(window);
        return NULL;
    }

    RAWINPUTDEVICE rid = {
        .usUsagePage = 0x01,          // HID_USAGE_PAGE_GENERIC
        .usUsage = 0x02,              // HID_USAGE_GENERIC_MOUSE
        .dwFlags = 0,//RIDEV_NOLEGACY,    // adds mouse and also ignores legacy mouse messages
        .hwndTarget = 0,
    };

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
        Window_Destroy(window);
        return NULL;
    }

    SetWindowLongPtrA(window->Handle, GWLP_USERDATA, cast(LONG_PTR) window);

    window->DeviceContext = GetDC(window->Handle);
    
    if (!window->DeviceContext) {
        Window_Destroy(window);
        return NULL;
    }

    PIXELFORMATDESCRIPTOR pixelFormatDescriptor = {
        .nSize = sizeof(pixelFormatDescriptor),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 32,
        .cDepthBits = 24,
        .cStencilBits = 8,
        .iLayerType = PFD_MAIN_PLANE,
    };

    int format = ChoosePixelFormat(window->DeviceContext, &pixelFormatDescriptor);
    if (!format) {
        Window_Destroy(window);
        return NULL;
    }

    if (!SetPixelFormat(window->DeviceContext, format, &pixelFormatDescriptor)) {
        Window_Destroy(window);
        return NULL;
    }

    window->GLContext = wglCreateContext(window->DeviceContext);

    if (!window->GLContext) {
        Window_Destroy(window);
        return NULL;
    }

    return window;
}

void Window_Destroy(Window* window) {
    if (window->Handle) {
        if (window->GLContext) {
            wglDeleteContext(window->GLContext);
        }

        if (window->DeviceContext) {
            ReleaseDC(window->Handle, window->DeviceContext);
        }

        DestroyWindow(window->Handle);
    }

    WindowCount--;
    if (WindowCount == 0) {
        UnregisterClassA(WindowClassName, window->Instance);
    }

    free(window);
}

b8 Window_MakeContextCurrent(Window* window) {
    return wglMakeCurrent(window->DeviceContext, window->GLContext) ? TRUE : FALSE;
}

void Window_Show(Window* window) {
    ShowWindow(window->Handle, SW_SHOW);
}

void Window_Hide(Window* window) {
    ShowWindow(window->Handle, SW_HIDE);
}

void Window_LockCursor(Window* window) {
    window->LockCursor = TRUE;
    GetClipCursor(&window->OldClip);
    RECT windowRect = {};
    GetWindowRect(window->Handle, &windowRect);
    RECT clipRect = {};
    SetRectEmpty(&clipRect);
    AdjustWindowRectEx(&clipRect, WindowStyle, 0, WindowStyleEx);
    windowRect.left -= clipRect.left + 10;
    windowRect.right -= clipRect.right + 10;
    windowRect.top -= clipRect.top + 10;
    windowRect.bottom -= clipRect.bottom + 10;
    ClipCursor(&windowRect);
    ShowCursor(FALSE);
}

void Window_UnlockCursor(Window* window) {
    window->LockCursor = FALSE;
    ClipCursor(&window->OldClip);
    ShowCursor(TRUE);
}

void Window_SetCloseCallback(Window* window, Window_CloseCallback callback, void* userData) {
    window->CloseCallback = callback;
    window->CloseCallbackUserData = userData;
}

void Window_SetResizeCallback(Window* window, Window_ResizeCallback callback, void* userData) {
    window->ResizeCallback = callback;
    window->ResizeCallbackUserData = userData;
}

void Window_SetKeyCallback(Window* window, Window_KeyCallback callback, void* userData) {
    window->KeyCallback = callback;
    window->KeyCallbackUserData = userData;
}

void Window_SetMouseMoveCallback(Window* window, Window_MouseMoveCallback callback, void* userData) {
    window->MouseMoveCallback = callback;
    window->MouseMoveCallbackUserData = userData;
}

b8 Window_PollEvents(Window* window) {
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) {
            return FALSE;
        }

        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return TRUE;
}

void Window_SwapBuffers(Window* window) {
    SwapBuffers(window->DeviceContext);
}
