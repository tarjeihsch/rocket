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

Platform::Window::Window()
    : m_NativeWindow(nullptr), 
    windowHandle(0)
{
}

Platform::Window::~Window()
{
    Destroy();
}

bool Platform::Window::Create(const WindowCreateInfo& createInfo)
{
    Destroy();

    glfwWindowHint(GLFW_RESIZABLE, createInfo.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, createInfo.visible ? GLFW_TRUE : GLFW_FALSE);

    GLFWwindow* glfwWindow = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title, nullptr, nullptr);

    if (!glfwWindow)
    {
        return false;
    }

    m_NativeWindow = glfwWindow;
    windowHandle = MakeWindowHandle(glfwWindow);

    WindowInfo windowInfo = {};

#if defined(_WIN32)
    windowInfo.serverType = WindowServerType::Win32;
    windowInfo.display = (void*)glfwGetWin32Window(m_NativeWindow);
    windowInfo.instance = (void*)GetModuleHandle(nullptr);
#elif defined(__linux__)
    #if defined(GLFW_NATIVE_WAYLAND)
        if (auto* wlDisplay = glfwGetWaylandDisplay(); wlDisplay != nullptr)
        {
            windowInfo.serverType = WindowServerType::Wayland;
            windowInfo.display = (void*)wlDisplay;
            windowInfo.instance = (void*)glfwGetWaylandWindow(static_cast<GLFWwindow*>(m_NativeWindow));
        }
        else
    #endif

    #if defined(GLFW_NATIVE_X11)
        {
            auto x11Window = glfwGetX11Window(static_cast<GLFWwindow*>(m_NativeWindow));
            windowInfo.serverType = WindowServerType::X11;
            windowInfo.display = (void*)glfwGetX11Display();
            windowInfo.instance = (void*)(uintptr_t)x11Window;
        }
    #else
        {
            static_assert("No support for Linux backend exposed.");
        }
    #endif
#elif defined(__APPLE__)
        windowInfo.serverType = WindowServerType::Cocoa;
        windowInfo.display = (void*)glfwGetCocoaWindow(m_NativeWindow);
        windowInfo.instance = nullptr;
#endif

    return true;
}

void Platform::Window::Destroy()
{
    if (!IsValid())
    {
        return;
    }

    UnregisterPlatformWindow(m_NativeWindow);

    glfwDestroyWindow(static_cast<GLFWwindow*>(m_NativeWindow));

    m_NativeWindow = nullptr;
    windowHandle = 0;
}

void Platform::Window::Show()
{
    if (!IsValid())
    {
        return;
    }

    glfwShowWindow(static_cast<GLFWwindow*>(m_NativeWindow));
}

void Platform::Window::Hide()
{
    if (!IsValid())
    {
        return;
    }

    glfwHideWindow(static_cast<GLFWwindow*>(m_NativeWindow));
}

void Platform::Window::SetTitle(const ANSICHAR* title)
{
    if (!IsValid())
    {
        return;
    }

    glfwSetWindowTitle(static_cast<GLFWwindow*>(m_NativeWindow), title);
}

Vector2i Platform::Window::GetSize() const
{
    if (!IsValid())
    {
        return Vector2i(-INT_MX, -INT_MX);
    }

    Vector2i result;
    glfwGetWindowSize(static_cast<GLFWwindow*>(m_NativeWindow), &result.data[0], &result.data[1]);
    return result;
}

Vector2i Platform::Window::GetPosition() const
{
    if (!IsValid())
    {
        return Vector2i(-INT_MX, -INT_MX);
    }

    Vector2i result;
    glfwGetWindowPos(static_cast<GLFWwindow*>(m_NativeWindow), &result.data[0], &result.data[1]);
    return result;
}
const WindowInfo& Platform::Window::GetWindowInfo() const
{
    return windowInfo;
}

uint64_t Platform::Window::GetHandle() const
{
    return windowHandle;
}

bool Platform::Window::IsValid() const
{
    return m_NativeWindow != nullptr;
}
