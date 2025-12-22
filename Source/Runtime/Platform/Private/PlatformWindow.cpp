#include "PlatformWindow.h"

#include <GLFW/glfw3.h>

namespace Platform
{
    /* Private function implemented in PlatformEvent.cpp - accessed as friend */
    void UnregisterPlatformWindow(void* nativeWindow);

    static uint64_t MakeWindowHandle(GLFWwindow* glfwWindow)
    {
        return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(glfwWindow));
    }

    Window::Window()
        : nativeWindow(nullptr), 
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

        nativeWindow = glfwWindow;
        windowHandle = MakeWindowHandle(glfwWindow);

        return true;
    }

    void Window::Destroy()
    {
        if (!IsValid())
        {
            return;
        }

        UnregisterPlatformWindow(nativeWindow);

        glfwDestroyWindow(static_cast<GLFWwindow*>(nativeWindow));

        nativeWindow = nullptr;
        windowHandle = 0;
    }

    void Window::Show()
    {
        if (!IsValid())
        {
            return;
        }

        glfwShowWindow(static_cast<GLFWwindow*>(nativeWindow));
    }

    void Window::Hide()
    {
        if (!IsValid())
        {
            return;
        }

        glfwHideWindow(static_cast<GLFWwindow*>(nativeWindow));
    }

    void Window::SetTitle(const ANSICHAR* title)
    {
        if (!IsValid())
        {
            return;
        }

        glfwSetWindowTitle(static_cast<GLFWwindow*>(nativeWindow), title);
    }

    Vector2i Window::GetSize() const
    {
        if (!IsValid())
        {
            return Vector2i(-INT_MX, -INT_MX);
        }

        Vector2i result;
        glfwGetWindowSize(static_cast<GLFWwindow*>(nativeWindow), &result.data[0], &result.data[1]);
        return result;
    }

    Vector2i Window::GetPosition() const
    {
        if (!IsValid())
        {
            return Vector2i(-INT_MX, -INT_MX);
        }

        Vector2i result;
        glfwGetWindowPos(static_cast<GLFWwindow*>(nativeWindow), &result.data[0], &result.data[1]);
        return result;
    }

    uint64_t Window::GetHandle() const
    {
        return windowHandle;
    }

    bool Window::IsValid() const
    {
        return nativeWindow != nullptr;
    }
}
