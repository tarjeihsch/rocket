#include "Window.h"

#include <GLFW/glfw3.h>

#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
    #define GLFW_EXPOSE_NATIVE_X11
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3native.h>

/* Private function implemented in PlatformEvent.cpp - accessed as friend */
void UnregisterPlatformWindow(void* nativeWindow);

static uint64_t MakeWindowHandle(GLFWwindow* glfwWindow)
{
    return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(glfwWindow));
}

OSWindow::OSWindow()
    : on_key_down(nullptr)
    , on_key_up(nullptr)
    , on_window_resize(nullptr)
	, on_mouse_move(nullptr)
	, on_mouse_move_delta(nullptr)
	, on_mouse_button_up(nullptr)
	, on_mouse_button_down(nullptr)
	, on_window_close(nullptr)
	, on_mouse_scroll(nullptr)
    , WindowHandle(nullptr)
    , WindowHandleID(0)
{
}

OSWindow::~OSWindow()
{
    Destroy();
}

bool OSWindow::Create(const WindowCreateInfo& createInfo)
{
    Destroy();

    glfwWindowHint(GLFW_RESIZABLE, createInfo.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, createInfo.visible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* glfwWindow = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title, nullptr, nullptr);

    if (!glfwWindow)
    {
        return false;
    }

    WindowHandle = glfwWindow;
    WindowHandleID = MakeWindowHandle(glfwWindow);

#if defined(_WIN32)
    NativeBackend = ENativePlatformBackend::Win32;
    NativeWindowHandle = (void*)glfwGetWin32Window(static_cast<GLFWwindow*>(WindowHandle));
    NativeWindowSystemHandle = (void*)GetModuleHandle(nullptr);
#elif defined(__linux__)
    NativeBackend = ENativePlatformBackend::X11;
    NativeWindowHandle = (void*)glfwGetX11Display();
    NativeWindowSystemHandle = (void*)(uintptr_t)glfwGetX11Window(static_cast<GLFWwindow*>(WindowHandle));;
#elif defined(__APPLE__)
    NativeBackend = WindowServerType::Cocoa;
    windowInfo.display = (void*)glfwGetCocoaWindow(WindowHandle);
    windowInfo.instance = nullptr;
#endif

    std::printf("[Platform] Spawn Window\n");
    std::printf("\tExtent: %ix%i\n", createInfo.width, createInfo.height);
    std::printf("\tBackend: %i\n", static_cast<uint8_t>(NativeBackend));

    return true;
}

void OSWindow::Destroy()
{
    if (!IsValid())
    {
        return;
    }

    UnregisterPlatformWindow(WindowHandle);

    glfwDestroyWindow(static_cast<GLFWwindow*>(WindowHandle));

    WindowHandle = nullptr;
    WindowHandleID = 0;
}

void OSWindow::Show()
{
    if (!IsValid())
    {
        return;
    }

    glfwShowWindow(static_cast<GLFWwindow*>(WindowHandle));
}

void OSWindow::Hide()
{
    if (!IsValid())
    {
        return;
    }

    glfwHideWindow(static_cast<GLFWwindow*>(WindowHandle));
}

void OSWindow::SetTitle(const ANSICHAR* title)
{
    if (!IsValid())
    {
        return;
    }

    glfwSetWindowTitle(static_cast<GLFWwindow*>(WindowHandle), title);
}

void OSWindow::WaitEvents()
{
    glfwWaitEvents();
}

Vector2i OSWindow::GetSize() const
{
    if (!IsValid())
    {
        return Vector2i(-INTEGER_MAX, -INTEGER_MAX);
    }

    Vector2i result;
    glfwGetWindowSize(static_cast<GLFWwindow*>(WindowHandle), &result.data[0], &result.data[1]);
    return result;
}

Vector2i OSWindow::GetPosition() const
{
    if (!IsValid())
    {
        return Vector2i(-INTEGER_MAX, -INTEGER_MAX);
    }

    Vector2i result;
    glfwGetWindowPos(static_cast<GLFWwindow*>(WindowHandle), &result.data[0], &result.data[1]);
    return result;
}

uint64_t OSWindow::GetWindowHandleID() const
{
    return WindowHandleID;
}

ENativePlatformBackend OSWindow::GetNativeBackend() const
{
    return NativeBackend;
}

RNativeWindowSystemHandle OSWindow::GetNativeWindowSystemHandle() const
{
    return NativeWindowSystemHandle;
}

RNativeWindowHandle OSWindow::GetNativeWindowHandle() const
{
    return NativeWindowHandle;
}

bool OSWindow::IsValid() const
{
    return WindowHandle != nullptr;
}

bool OSWindow::ShouldClose() const
{
    return glfwWindowShouldClose(static_cast<GLFWwindow*>(WindowHandle));
}

void OSWindow::RequestClose()
{
    glfwSetWindowShouldClose(static_cast<GLFWwindow*>(WindowHandle), GLFW_TRUE);
}

bool OSWindow::IsFocused() const
{
    return glfwGetWindowAttrib(static_cast<GLFWwindow*>(WindowHandle), GLFW_FOCUSED);
}