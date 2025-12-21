#include "GLFWWindow.h"

#include "Assertion.h"
#include "PlatformTypes.h"

#include <GLFW/glfw3.h>

RGLFWWindow::RGLFWWindow()
    : NativeWindow(nullptr), bIsMinimized(0), bIsFocused(0)
{
}

void RGLFWWindow::CreateNativeWindow(RWindowInitParams& InitParams)
{
    glfwSetErrorCallback([](int Code, const char* Desc)
    {
        RK_LOG_ERROR("(GLFW) Window Error: \n\tCode: {}\n\tDescription:{}", Code, Desc);
    });

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    NativeWindow = glfwCreateWindow(InitParams.Width, InitParams.Height, InitParams.Title, nullptr, nullptr);
    ensure(NativeWindow, "Failed to create GLFW window");

    glfwSetWindowUserPointer((GLFWwindow*)NativeWindow, this);

    glfwSetKeyCallback((GLFWwindow*)NativeWindow, [](GLFWwindow* Window, int32_t KeyCode, int32_t ScanCode, int32_t Action, int32_t Mod)
    {
	if (KeyCode == RK_KEY_ESCAPE)
	{
	    glfwSetWindowShouldClose(Window, true);
	}
    });

    glfwSetFramebufferSizeCallback((GLFWwindow*)NativeWindow, [](GLFWwindow* GlfwWindow, int Width, int Height)
    {
	RGLFWWindow* Window = static_cast<RGLFWWindow*>(glfwGetWindowUserPointer(GlfwWindow));

	// Handle edgecase where framebuffer size is 0 (minimized)
	if (Width == 0 || Height == 0)
	{
	    if (!Window->IsMinimized()) Window->SetIsMinimized(true);
	}
	else
	{
	    if (Window->IsMinimized()) Window->SetIsMinimized(false);
	}

	while (Window->IsMinimized())
	{
	    // Rough handling of window minimization.
	    glfwWaitEvents();
	}

        //GetRHI()->Resize();
    });

    glfwSetWindowFocusCallback((GLFWwindow*)NativeWindow, [](GLFWwindow* GlfwWindow, int32_t bFocused)
    {
	RGLFWWindow* Window = static_cast<RGLFWWindow*>(glfwGetWindowUserPointer(GlfwWindow));

	if (bFocused)
	{
	    Window->OnWindowFocusDelegate.Broadcast(bFocused);
	    Window->SetIsFocused(true);
	}
	else
	{
	    Window->OnWindowFocusDelegate.Broadcast(bFocused);
	    Window->SetIsFocused(false);
	}
    });

    glfwSetWindowPosCallback((GLFWwindow*)NativeWindow, [](GLFWwindow* GlfwWindow, int32_t InPositionX, int32_t InPositionY)
    {
	RGLFWWindow* Window = static_cast<RGLFWWindow*>(glfwGetWindowUserPointer(GlfwWindow));
    });

    OnWindowFocusDelegate.Broadcast(1);
    SetIsFocused(true);
}

void RGLFWWindow::DestroyNativeWindow()
{
    glfwDestroyWindow((GLFWwindow*)NativeWindow);
    glfwTerminate();
}

void RGLFWWindow::Poll()
{
    glfwPollEvents();
}

bool RGLFWWindow::ShouldClose() const
{
    return glfwWindowShouldClose((GLFWwindow*)NativeWindow);
}

bool RGLFWWindow::IsMinimized() const
{
    return bIsMinimized;
}

void RGLFWWindow::SetIsMinimized(bool InMinimized)
{
    bIsMinimized = InMinimized;
}

void RGLFWWindow::SetIsFocused(bool bFocused)
{
    bIsFocused = bFocused;
}

int32_t RGLFWWindow::GetWindowWidth() const
{
    int32_t Width;
    glfwGetWindowSize(static_cast<GLFWwindow*>(NativeWindow), &Width, nullptr);
    return Width;
}

int32_t RGLFWWindow::GetWindowHeight() const
{
    int32_t Height;
    glfwGetWindowSize(static_cast<GLFWwindow*>(NativeWindow), nullptr, &Height);
    return Height;
}

int32_t RGLFWWindow::GetFramebufferWidth() const
{
    int32_t Width;
    glfwGetFramebufferSize((GLFWwindow*)GWindow->GetNativeWindow(), &Width, nullptr);
    return Width;
}

int32_t RGLFWWindow::GetFramebufferHeight() const
{
    int32_t Height;
    glfwGetFramebufferSize((GLFWwindow*)GWindow->GetNativeWindow(), nullptr, &Height);
    return Height;
}

void RGLFWWindow::WaitEventOrTimeout(float TimeoutSeconds)
{
    glfwWaitEventsTimeout(TimeoutSeconds);
}

float RGLFWWindow::GetAspectRatio() const
{
    return static_cast<float>(GetFramebufferWidth()) / static_cast<float>(GetFramebufferHeight());
}
void *RGLFWWindow::GetNativeWindow() const
{
    return NativeWindow;
}

bool RGLFWWindow::IsFocused() const
{
    return bIsFocused;
}
