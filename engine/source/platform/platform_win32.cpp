#include "platform/platform.hpp"

#if FBPLATFORM_WINDOWS

#include "core/logger.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>

#include <stdlib.h>

using namespace fabric;

namespace {
    struct internal_state {
        HINSTANCE hinstance;
        HWND hwnd;
    };

    static f64 clock_frequency;
    static LARGE_INTEGER start_time;
}  // namespace

LRESULT CALLBACK win32_process_message(HWND hWnd, u32 msg, WPARAM wParam, LPARAM lParam);

b8 initialize(fabric::platform::state* platformState, const char* applicationName, i32 x, i32 y, i32 width, i32 height) {
    platformState->internal_state = malloc(sizeof(internal_state));
    internal_state* internal = (internal_state*)platformState->internal_state;

    internal->hinstance = GetModuleHandleA(0);

    WNDCLASSA wc{};
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = win32_process_message;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = internal->hinstance;
    wc.hIcon = LoadIcon(internal->hinstance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(internal->hinstance, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "window_class";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    u32 windowX = x;
    u32 windowY = y;
    u32 window_width = width;
    u32 window_height = height;

    u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 extended_window_style = WS_EX_APPWINDOW;

    window_style |= WS_MAXIMIZEBOX;
    window_style |= WS_MINIMIZEBOX;
    window_style |= WS_THICKFRAME;

    RECT border_rect{};
    AdjustWindowRectEx(&border_rect, window_style, 0, extended_window_style);

    windowX += border_rect.left;
    windowY += border_rect.top;
    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    HWND handle = CreateWindowExA(extended_window_style, wc.lpszClassName, applicationName, window_style,
                                  windowX, windowY, window_width, window_height, 0, 0, internal->hinstance, 0);

    if (!handle) {
        MessageBoxA(0, "Window creation failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    internal->hwnd = handle;

    b8 should_activate = true;  // TODO: If the window shouldn't accept input, this should be false;
    i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;

    ShowWindow(internal->hwnd, show_window_command_flags);

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);

    return true;
}

void shutdown(fabric::platform::state* platformState) {
    internal_state* internal = (internal_state*)platformState->internal_state;

    if (internal->hwnd) {
        DestroyWindow(internal->hwnd);
        internal->hwnd = nullptr;
    }
}

b8 pump_messages(fabric::platform::state* platformState) {
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return true;
}

void* platform::allocate_memory(u64 size, b8 aligned) {
    return malloc(size);
}

void platform::free_memory(void* block, b8 aligned) {
    free(block);
}

void* platform::zero_memory(void* block, u64 size) {
    return set_memory(block, 0, size);
}

void* platform::copy_memory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* platform::set_memory(void* dest, i32 value, u64 size) {
    return memset(dest, value, size);
}

void platform::console_write(const char* message, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    static u8 levels[logger::LOG_LEVEL_COUNT] = {64, 4, 6, 2, 1};
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(console_handle, message, (DWORD)length, number_written, 0);
}

void platform::console_write_error(const char* message, u8 color) {
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    static u8 levels[logger::LOG_LEVEL_COUNT] = {64, 4, 6, 2, 1};
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(console_handle, message, (DWORD)length, number_written, 0);
}

f64 platform::get_absolute_time() {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    return (f64)now.QuadPart * clock_frequency;
}

void platform::sleep(u64 ms) {
    Sleep(ms);
}

LRESULT CALLBACK win32_process_message(HWND hWnd, u32 msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_ERASEBKGND:
            // Notify the OS that erasing the screen will be handled by the application to prevent flicker.
            return 1;

        case WM_CLOSE:
            // TODO: Fire an event for the application to quit.
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE: {
            // Get the updated size
            // RECT rect;
            // GetClientRect(hWnd, &rect);
            // u32 width = rect.right - rect.left;
            // u32 height = rect.bottom - rect.top;

            // TODO: Fire an event for window resize.
        } break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);

            // TODO: input processing.
        } break;

        case WM_MOUSEMOVE: {
            // i32 xPosition = GET_X_LPARAM(lParam);
            // i32 yPosition = GET_Y_LPARAM(lParam);

            // TODO: input processing.
        } break;

        case WM_MOUSEWHEEL: {
            // i32 wheel_delta = GET_WHEEL_DELTA_WPARAM(wParam);

            // if(wheel_delta != 0) {
            //     wheel_delta = (wheel_delta < 0) ? -1 : 1;
            // }

            // TODO: input processing.
        } break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP: {
            // b8 pressed = (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg = WM_MBUTTONDOWN);

            // TODO: input processing.
        } break;
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

#endif