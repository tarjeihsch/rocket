#include "PlatformEvent.h"

#include "Assertion.h"

#include <GLFW/glfw3.h>
#include <cstring>

#define GLFW_KEY_SPACE              32
#define GLFW_KEY_APOSTROPHE         39  /* ' */
#define GLFW_KEY_COMMA              44  /* , */
#define GLFW_KEY_MINUS              45  /* - */
#define GLFW_KEY_PERIOD             46  /* . */
#define GLFW_KEY_SLASH              47  /* / */
#define GLFW_KEY_0                  48
#define GLFW_KEY_1                  49
#define GLFW_KEY_2                  50
#define GLFW_KEY_3                  51
#define GLFW_KEY_4                  52
#define GLFW_KEY_5                  53
#define GLFW_KEY_6                  54
#define GLFW_KEY_7                  55
#define GLFW_KEY_8                  56
#define GLFW_KEY_9                  57
#define GLFW_KEY_SEMICOLON          59  /* ; */
#define GLFW_KEY_EQUAL              61  /* = */
#define GLFW_KEY_A                  65
#define GLFW_KEY_B                  66
#define GLFW_KEY_C                  67
#define GLFW_KEY_D                  68
#define GLFW_KEY_E                  69
#define GLFW_KEY_F                  70
#define GLFW_KEY_G                  71
#define GLFW_KEY_H                  72
#define GLFW_KEY_I                  73
#define GLFW_KEY_J                  74
#define GLFW_KEY_K                  75
#define GLFW_KEY_L                  76
#define GLFW_KEY_M                  77
#define GLFW_KEY_N                  78
#define GLFW_KEY_O                  79
#define GLFW_KEY_P                  80
#define GLFW_KEY_Q                  81
#define GLFW_KEY_R                  82
#define GLFW_KEY_S                  83
#define GLFW_KEY_T                  84
#define GLFW_KEY_U                  85
#define GLFW_KEY_V                  86
#define GLFW_KEY_W                  87
#define GLFW_KEY_X                  88
#define GLFW_KEY_Y                  89
#define GLFW_KEY_Z                  90
#define GLFW_KEY_LEFT_BRACKET       91  /* [ */
#define GLFW_KEY_BACKSLASH          92  /* \ */
#define GLFW_KEY_RIGHT_BRACKET      93  /* ] */
#define GLFW_KEY_GRAVE_ACCENT       96  /* ` */
#define GLFW_KEY_WORLD_1            161 /* non-US #1 */
#define GLFW_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define GLFW_KEY_ESCAPE             256
#define GLFW_KEY_ENTER              257
#define GLFW_KEY_TAB                258
#define GLFW_KEY_BACKSPACE          259
#define GLFW_KEY_INSERT             260
#define GLFW_KEY_DELETE             261
#define GLFW_KEY_RIGHT              262
#define GLFW_KEY_LEFT               263
#define GLFW_KEY_DOWN               264
#define GLFW_KEY_UP                 265
#define GLFW_KEY_PAGE_UP            266
#define GLFW_KEY_PAGE_DOWN          267
#define GLFW_KEY_HOME               268
#define GLFW_KEY_END                269
#define GLFW_KEY_CAPS_LOCK          280
#define GLFW_KEY_SCROLL_LOCK        281
#define GLFW_KEY_NUM_LOCK           282
#define GLFW_KEY_PRINT_SCREEN       283
#define GLFW_KEY_PAUSE              284
#define GLFW_KEY_F1                 290
#define GLFW_KEY_F2                 291
#define GLFW_KEY_F3                 292
#define GLFW_KEY_F4                 293
#define GLFW_KEY_F5                 294
#define GLFW_KEY_F6                 295
#define GLFW_KEY_F7                 296
#define GLFW_KEY_F8                 297
#define GLFW_KEY_F9                 298
#define GLFW_KEY_F10                299
#define GLFW_KEY_F11                300
#define GLFW_KEY_F12                301
#define GLFW_KEY_LEFT_SHIFT         340
#define GLFW_KEY_LEFT_CONTROL       341
#define GLFW_KEY_LEFT_ALT           342
#define GLFW_KEY_LEFT_SUPER         343
#define GLFW_KEY_RIGHT_SHIFT        344
#define GLFW_KEY_RIGHT_CONTROL      345
#define GLFW_KEY_RIGHT_ALT          346
#define GLFW_KEY_RIGHT_SUPER        347

namespace Platform
{
    static double lastMouseX = 0.0;
    static double lastMouseY = 0.0;

    struct NativeWindowUserData
    {
        uint64_t windowHandle;
        EventSink* eventSink;
    };

    static NativeWindowUserData* GetUserData(GLFWwindow* window)
    { 
        NativeWindowUserData* userData = static_cast<NativeWindowUserData*>(glfwGetWindowUserPointer(window));
        ensure(userData != nullptr);
        return userData;
    }

    static Modifier GetModifier(int32_t modifier)
    {
        Modifier result;
        result.isShiftDown = (modifier & GLFW_MOD_SHIFT) != 0;
        result.isCtrlDown = (modifier & GLFW_MOD_CONTROL) != 0;
        result.isAltDown = (modifier & GLFW_MOD_ALT) != 0;
        result.isMetaDown = (modifier & GLFW_MOD_SUPER) != 0;

        return result;
    }

    static Button GetButton(int32_t button)
    {
        switch (button)
        {
            case GLFW_MOUSE_BUTTON_LEFT: return Button::Left;
            case GLFW_MOUSE_BUTTON_MIDDLE: return Button::Middle;
            case GLFW_MOUSE_BUTTON_RIGHT: return Button::Right;
            case GLFW_MOUSE_BUTTON_4: return Button::X1;
            case GLFW_MOUSE_BUTTON_5: return Button::X2;
            default: return Button::Unknown;
        }
    }

    static Key GetKey(int32_t key)
    {
        switch (key)
        {
			case GLFW_KEY_A: return Key::A;
			case GLFW_KEY_B: return Key::B;
			case GLFW_KEY_C: return Key::C;
			case GLFW_KEY_D: return Key::D;
			case GLFW_KEY_E: return Key::E;
			case GLFW_KEY_F: return Key::F;
			case GLFW_KEY_G: return Key::G;
			case GLFW_KEY_H: return Key::H;
			case GLFW_KEY_I: return Key::I;
			case GLFW_KEY_J: return Key::J;
			case GLFW_KEY_K: return Key::K;
			case GLFW_KEY_L: return Key::L;
			case GLFW_KEY_M: return Key::M;
			case GLFW_KEY_N: return Key::N;
			case GLFW_KEY_O: return Key::O;
			case GLFW_KEY_P: return Key::P;
			case GLFW_KEY_Q: return Key::Q;
			case GLFW_KEY_R: return Key::R;
			case GLFW_KEY_S: return Key::S;
			case GLFW_KEY_T: return Key::T;
			case GLFW_KEY_U: return Key::U;
			case GLFW_KEY_V: return Key::V;
			case GLFW_KEY_W: return Key::W;
			case GLFW_KEY_X: return Key::X;
			case GLFW_KEY_Y: return Key::Y;
			case GLFW_KEY_Z: return Key::Z;
			case GLFW_KEY_0: return Key::Num0;
			case GLFW_KEY_1: return Key::Num1;
			case GLFW_KEY_2: return Key::Num2;
			case GLFW_KEY_3: return Key::Num3;
			case GLFW_KEY_4: return Key::Num4;
			case GLFW_KEY_5: return Key::Num5;
			case GLFW_KEY_6: return Key::Num6;
			case GLFW_KEY_7: return Key::Num7;
			case GLFW_KEY_8: return Key::Num8;
			case GLFW_KEY_9: return Key::Num9;
			case GLFW_KEY_ESCAPE: return Key::Escape;
			case GLFW_KEY_ENTER: return Key::Enter;
			case GLFW_KEY_TAB: return Key::Tab;
			case GLFW_KEY_BACKSPACE: return Key::Backspace;
			case GLFW_KEY_SPACE: return Key::Space;

			case GLFW_KEY_LEFT: return Key::Left;
			case GLFW_KEY_RIGHT: return Key::Right;
			case GLFW_KEY_UP: return Key::Up;
			case GLFW_KEY_DOWN: return Key::Down;

			case GLFW_KEY_INSERT: return Key::Insert;
			case GLFW_KEY_DELETE: return Key::Delete;
			case GLFW_KEY_HOME: return Key::Home;
			case GLFW_KEY_END: return Key::End;
			case GLFW_KEY_PAGE_UP: return Key::PageUp;
			case GLFW_KEY_PAGE_DOWN: return Key::PageDown;

			case GLFW_KEY_LEFT_SHIFT: return Key::LeftShift;
			case GLFW_KEY_RIGHT_SHIFT: return Key::RightShift;
			case GLFW_KEY_LEFT_CONTROL: return Key::LeftCtrl;
			case GLFW_KEY_RIGHT_CONTROL: return Key::RightCtrl;
			case GLFW_KEY_LEFT_ALT:  return Key::LeftAlt;
			case GLFW_KEY_RIGHT_ALT: return Key::RightAlt;
			case GLFW_KEY_LEFT_SUPER: return Key::LeftMeta;
			case GLFW_KEY_RIGHT_SUPER: return Key::RightMeta;

			case GLFW_KEY_F1: return Key::F1;
			case GLFW_KEY_F2: return Key::F2;
			case GLFW_KEY_F3: return Key::F3;
			case GLFW_KEY_F4: return Key::F4;
			case GLFW_KEY_F5: return Key::F5;
			case GLFW_KEY_F6: return Key::F6;
			case GLFW_KEY_F7: return Key::F7;
			case GLFW_KEY_F8: return Key::F8;
			case GLFW_KEY_F9: return Key::F9;
			case GLFW_KEY_F10: return Key::F10;
			case GLFW_KEY_F11: return Key::F11;
			case GLFW_KEY_F12: return Key::F12;

			default: return Key::Unknown;
        }
    }

    static void Enqueue(EventSink& eventSink, const Event& event)
    {
        eventSink.Enqueue(event);
    }

    static void OnWindowClose(GLFWwindow* glfwWindow)
    {
        NativeWindowUserData* userData = GetUserData(glfwWindow);

        Event event = {};
        event.windowHandle = userData->windowHandle;
        event.type = EventType::WindowCloseRequested;

        Enqueue(*userData->eventSink, event);
    }


    static void OnWindowSize(GLFWwindow* glfwWindow, int32_t width, int32_t height)
    {
        NativeWindowUserData* userData = GetUserData(glfwWindow);

        Event event = {};
        event.windowHandle = userData->windowHandle;
        event.type = EventType::WindowResized;
        event.windowResize.value.x = width;
        event.windowResize.value.y = height;

        Enqueue(*userData->eventSink, event);
    }

    static void OnWindowMove(GLFWwindow* glfwWindow, int32_t x, int32_t y)
    {
        NativeWindowUserData* userData = GetUserData(glfwWindow);

        Event event = {};
        event.windowHandle = userData->windowHandle;
        event.type = EventType::WindowMoved;
        event.windowMove.value.x = x;
        event.windowMove.value.y = y;

        Enqueue(*userData->eventSink, event);
    }

    static void OnWindowFocus(GLFWwindow* glfwWindow, int32_t focusState)
    {
        NativeWindowUserData* userData = GetUserData(glfwWindow);

        Event event = {};
        event.windowHandle = userData->windowHandle;

        if (focusState == GLFW_TRUE)
        {
            event.type = EventType::WindowFocusGained;
        }
        else if (focusState == GLFW_FALSE)
        {
            event.type = EventType::WindowFocusLost;
        }

        Enqueue(*userData->eventSink, event);
    }

    
    static void OnKey(GLFWwindow* glfwWindow, int32_t key, int32_t scanCode, int32_t action, int32_t modifier)
    {
        NativeWindowUserData* userData = GetUserData(glfwWindow);

        Event event = {};
        event.windowHandle = userData->windowHandle;

        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            event.type = EventType::KeyDown;
        }
        else if (action == GLFW_RELEASE)
        {
            event.type = EventType::KeyUp;
        }

        event.key.value = GetKey(key);
        event.key.modifier = GetModifier(modifier);
        event.key.repeat = action == GLFW_REPEAT;

        Enqueue(*userData->eventSink, event);
    }

    static void OnText(GLFWwindow* glfwWindow, uint32_t codepoint)
    {
        NativeWindowUserData* userData = GetUserData(glfwWindow);

        Event event = {};
        event.windowHandle = userData->windowHandle;
        event.type = EventType::TextInput;

        ANSICHAR* buffer = event.text.value.buffer;
        if (codepoint <= 0x7F)
        {
            buffer[0] = static_cast<ANSICHAR>(codepoint);
            buffer[1] = 0;
        }
        else if (codepoint <= 0x7FF)
        {
            buffer[0] = static_cast<ANSICHAR>(0xC0 | ((codepoint >> 6) & 0x1F));
            buffer[1] = static_cast<ANSICHAR>(0x80 | (codepoint & 0x3F));
            buffer[2] = 0;
        }
        else if (codepoint <= 0x7FFFF)
        {
            buffer[0] = static_cast<ANSICHAR>(0xE0 | ((codepoint >> 12) & 0x0F));
            buffer[1] = static_cast<ANSICHAR>(0x80 | ((codepoint >> 6) & 0x3F));
            buffer[2] = static_cast<ANSICHAR>(0x80 | (codepoint & 0x3F));
            buffer[3] = 0;
        }
        else
        {
            buffer[0] = static_cast<ANSICHAR>(0xF0 | ((codepoint >> 18) & 0x7F));
            buffer[1] = static_cast<ANSICHAR>(0xE0 | ((codepoint >> 12) & 0x3F));
            buffer[2] = static_cast<ANSICHAR>(0x80 | ((codepoint >> 6) & 0x3F));
            buffer[3] = static_cast<ANSICHAR>(0x80 | (codepoint & 0x3F));
            buffer[4] = 0;
        }

        Enqueue(*userData->eventSink, event);
    }

    static void OnMouseMove(GLFWwindow* glfwWindow, double mouseX, double mouseY)
    {
        NativeWindowUserData* userData = GetUserData(glfwWindow);

        Event event = {};
        event.windowHandle = userData->windowHandle;
        event.type = EventType::MouseMove;
        event.axis.position.x = static_cast<int32_t>(mouseX);
        event.axis.position.y = static_cast<int32_t>(mouseY);
        event.axis.delta.x = static_cast<int32_t>(mouseX - lastMouseX);
        event.axis.delta.y = static_cast<int32_t>(mouseY - lastMouseY);

        // Cache mouse movement for delta calculation
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        Enqueue(*userData->eventSink, event);
    }

    static void OnMouseButton(GLFWwindow* glfwWindow, int32_t button, int32_t action, int32_t modifier)
    {
        NativeWindowUserData* userData = GetUserData(glfwWindow);

        Event event = {};
        event.windowHandle = userData->windowHandle;

        if (action == GLFW_PRESS)
        {
            event.type = EventType::MouseButtonDown;
        }
        else if (action == GLFW_RELEASE)
        {
            event.type = EventType::MouseButtonUp;
        }

        event.button.value = GetButton(button);
        event.button.modifier = GetModifier(modifier);
        event.button.position.x = static_cast<int32_t>(lastMouseX);
        event.button.position.y = static_cast<int32_t>(lastMouseY);

        Enqueue(*userData->eventSink, event);
    }

    static void OnMouseScroll(GLFWwindow* glfwWindow, double xOffset, double yOffset)
    {
        NativeWindowUserData* userData = GetUserData(glfwWindow);

        Event event = {};
        event.windowHandle = userData->windowHandle;
        event.type = EventType::MouseWheel;
        event.wheel.value.x = static_cast<float>(xOffset);
        event.wheel.value.y = static_cast<float>(yOffset);

        Enqueue(*userData->eventSink, event);
    }

    void RegisterPlatformWindow(void* nativeWindow, uint64_t windowHandle, EventSink* eventSink)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(nativeWindow);
        
        ensure(window);
        ensure(eventSink);

        NativeWindowUserData* userData = new NativeWindowUserData();
        userData->windowHandle = windowHandle;
        userData->eventSink = eventSink;

        glfwSetWindowUserPointer(window, userData);

        glfwSetWindowCloseCallback(window, &OnWindowClose);
        glfwSetWindowSizeCallback(window, &OnWindowSize);
        glfwSetWindowPosCallback(window, &OnWindowMove);
        glfwSetWindowFocusCallback(window, &OnWindowFocus);

        glfwSetKeyCallback(window, &OnKey);
        glfwSetCharCallback(window, &OnText);

        glfwSetCursorPosCallback(window, &OnMouseMove);
        glfwSetMouseButtonCallback(window, &OnMouseButton);
        glfwSetScrollCallback(window, &OnMouseScroll);
    }

    void UnregisterPlatformWindow(void* nativeWindow)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(nativeWindow);
        NativeWindowUserData* userData = GetUserData(window);
        
        ensure(window);
        ensure(userData);
        
        glfwSetWindowCloseCallback(window, nullptr);
        glfwSetWindowSizeCallback(window, nullptr);
        glfwSetWindowPosCallback(window, nullptr);
        glfwSetWindowFocusCallback(window, nullptr);

        glfwSetKeyCallback(window, nullptr);
        glfwSetCharCallback(window, nullptr);

        glfwSetCursorPosCallback(window, nullptr);
        glfwSetMouseButtonCallback(window, nullptr);
        glfwSetScrollCallback(window, nullptr);

        glfwSetWindowUserPointer(window, nullptr);
        delete userData;
    }
}
