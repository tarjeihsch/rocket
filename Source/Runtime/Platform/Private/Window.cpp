#include "Window.h"

#include <GLFW/glfw3.h>

/* Private function implemented in PlatformEvent.cpp - accessed as friend */
void UnregisterPlatformWindow(void* nativeWindow);

static uint64_t MakeWindowHandle(GLFWwindow* glfwWindow)
{
    return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(glfwWindow));
}

Window::Window()
    : m_NativeWindow(nullptr), 
    windowHandle(0)
{
}

Window::~Window()
{
    Destroy();
}

bool Window::Create(const WindowCreateInfo& createInfo)
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

    return true;
}

void Window::Destroy()
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

void Window::Show()
{
    if (!IsValid())
    {
        return;
    }

    glfwShowWindow(static_cast<GLFWwindow*>(m_NativeWindow));
}

void Window::Hide()
{
    if (!IsValid())
    {
        return;
    }

    glfwHideWindow(static_cast<GLFWwindow*>(m_NativeWindow));
}

void Window::SetTitle(const ANSICHAR* title)
{
    if (!IsValid())
    {
        return;
    }

    glfwSetWindowTitle(static_cast<GLFWwindow*>(m_NativeWindow), title);
}

Vector2i Window::GetSize() const
{
    if (!IsValid())
    {
        return Vector2i(-INT_MX, -INT_MX);
    }

    Vector2i result;
    glfwGetWindowSize(static_cast<GLFWwindow*>(m_NativeWindow), &result.data[0], &result.data[1]);
    return result;
}

Vector2i Window::GetPosition() const
{
    if (!IsValid())
    {
        return Vector2i(-INT_MX, -INT_MX);
    }

    Vector2i result;
    glfwGetWindowPos(static_cast<GLFWwindow*>(m_NativeWindow), &result.data[0], &result.data[1]);
    return result;
}

uint64_t Window::GetHandle() const
{
    return windowHandle;
}

bool Window::IsValid() const
{
    return m_NativeWindow != nullptr;
}