#include "GLFWInput.h"

#include "KeyCode.h"
#include <GLFW/glfw3.h>
#include "Window.h"

#if defined(RK_PLATFORM_WINDOWS) || defined(RK_PLATFORM_LINUX)
PInput* PInput::GInput = new CGLFWInput();
#endif

bool CGLFWInput::GetKeyPressedImpl(int32_t KeyCode)
{
    auto Window = static_cast<GLFWwindow*>(IWindow::Get()->GetNativeWindow());
    auto State = glfwGetKey(Window, KeyCode);

    return State == GLFW_PRESS || State == GLFW_REPEAT;
}

bool CGLFWInput::GetMouseButtonPressedImpl(int32_t KeyCode)
{
    auto Window = static_cast<GLFWwindow*>(IWindow::Get()->GetNativeWindow());
    auto State = glfwGetMouseButton(Window, KeyCode);

    return State == GLFW_PRESS;
}

float CGLFWInput::GetMouseXImpl()
{
    auto[MouseX, MouseY] = GetMousePositionImpl();
    return MouseX;
}

float CGLFWInput::GetMouseYImpl()
{
    auto[MouseX, MouseY] = GetMousePositionImpl();
    return MouseY;
}

std::pair<float, float> CGLFWInput::GetMousePositionImpl()
{
    auto Window = static_cast<GLFWwindow*>(IWindow::Get()->GetNativeWindow());
    double MouseX, MouseY;
    glfwGetCursorPos(Window, &MouseX, &MouseY);
    return { (float)MouseX, (float)MouseY };
}

int32_t CGLFWInput::GetFirstJoystickIDImpl() const
{
    for (int32_t JoystickID = RK_JOYSTICK_DEVICE_1; JoystickID <= RK_JOYSTICK_DEVICE_LAST; ++JoystickID)
    {
        if (glfwJoystickPresent(JoystickID))
        {
            return JoystickID;
        }
    }
    return -1;
}

bool CGLFWInput::GetJoystickButtonImpl(const int32_t JoystickID, const int32_t Button)
{
    int32_t ButtonCount;
    const unsigned char* Buttons = glfwGetJoystickButtons(JoystickID, &ButtonCount);
    if (Button < ButtonCount)
    {
        return Buttons[Button] == GLFW_PRESS;
    }
    return false;
}

float CGLFWInput::GetJoystickAxisImpl(const int32_t JoystickID, const int32_t Axis)
{
    int32_t AxisCount;
    const float* Axes = glfwGetJoystickAxes(JoystickID, &AxisCount);
    const float Value = Axes[Axis];
    return Value;
}
