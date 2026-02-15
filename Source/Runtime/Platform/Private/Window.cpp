#include "Window.h"

#include <GLFW/glfw3.h>

#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_WAYLAND
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
    : WindowHandle(nullptr), 
    WindowHandleID(0)
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
    #if defined(GLFW_EXPOSE_NATIVE_WAYLAND)
        if (auto* wlDisplay = glfwGetWaylandDisplay(); wlDisplay != nullptr)
        {
            NativeBackend = ENativePlatformBackend::Wayland;
            NativeWindowHandle = (void*)wlDisplay;
            NativeWindowSystemHandle = (void*)glfwGetWaylandWindow(static_cast<GLFWwindow*>(WindowHandle));
        }
        else
    #endif
    #if defined(GLFW_EXPOSE_NATIVE_X11)
        {
            auto x11Window = glfwGetX11Window(static_cast<GLFWwindow*>(WindowHandle));
            NativeBackend = ENativePlatformBackend::X11;
            NativeWindowHandle = (void*)glfwGetX11Display();
            NativeWindowSystemHandle = (void*)(uintptr_t)x11Window;
        }
    #else
        {
            static_assert("No support for Linux backend exposed.");
        }
    #endif
#elif defined(__APPLE__)
    NativeBackend = WindowServerType::Cocoa;
        windowInfo.display = (void*)glfwGetCocoaWindow(WindowHandle);
        windowInfo.instance = nullptr;
#endif

    //RK_LOG_INFO("[Platform] Spawn window");
    //RK_LOG_INFO("[Platform] \t\tExtent: {}x{}", createInfo.width, createInfo.height);
    //RK_LOG_INFO("[Platform] \t\tBackend: {}", static_cast<uint8_t>(NativeBackend));

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

Vector2i OSWindow::GetSize() const
{
    if (!IsValid())
    {
        return Vector2i(-INT_MX, -INT_MX);
    }

    Vector2i result;
    glfwGetWindowSize(static_cast<GLFWwindow*>(WindowHandle), &result.data[0], &result.data[1]);
    return result;
}

Vector2i OSWindow::GetPosition() const
{
    if (!IsValid())
    {
        return Vector2i(-INT_MX, -INT_MX);
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
