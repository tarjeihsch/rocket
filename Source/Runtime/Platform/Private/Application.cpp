#include "Application.h"

#include "CoreTypes.h"
#include "Event.h"
#include "Window.h"
#include "MessageController.h"
#include "EventSink.h"

#include <mutex>
#include <deque>
#include <GLFW/glfw3.h>

void RegisterPlatformWindow(void* nativeWindow, uint64_t WindowHandleID, EventSink* eventSink);

class EventQueue : public EventSink
{
public:
    void Enqueue(const Event& event) override
    {
        std::scoped_lock lock(mutex);
        queue.push_back(event);
    }

    bool Dequeue(Event& event)
    {
        std::scoped_lock lock(mutex);
        if (queue.empty())
        {
            return false;
        }

        event = queue.front();
        queue.pop_front();
        return true;
    }

    void Clear()
    {
        std::scoped_lock lock(mutex);
        queue.clear();
    }

private:
    std::mutex mutex;
    std::deque<Event> queue;
};

static EventQueue eventQueue;

Application::Application()
{
    message_controller = nullptr;
    M_IsValid = false;
}

Application& Application::Get()
{
    static Application platform;
    return platform;
}

bool Application::Initialize()
{
    if (M_IsValid)
    {
        return true;
    }

#if defined(_WIN32)
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WIN32);
#endif

#if defined(__linux__)
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

    if (glfwInit() != GLFW_TRUE)
    {
        return false;
    }

    M_IsValid = true;
    return true;
}

void Application::Shutdown()
{
    if (!M_IsValid)
    {
        return;
    }

    eventQueue.Clear();

    M_Windows.clear();
    M_IsValid = false;

    glfwTerminate();
}

void Application::PumpMessageQueue()
{
    glfwPollEvents();

    Event event = {};
    while (eventQueue.Dequeue(event))
    {
        ProcessDeferredEvent(event);
    }
}

void Application::WaitForEvents(double timeout)
{
    glfwWaitEventsTimeout(timeout);
}

void Application::ProcessDeferredEvent(const Event &event)
{
    OSWindow* window = FindWindowByHandle(event.WindowHandleID);
    
    switch (event.type)
    {
        case EventType::Quit:
        {
            //message_controller->OnExit();
            break;
        }

        case EventType::WindowCloseRequested:
        {
            //message_controller->OnWindowClose(window);
            window->on_window_close();
            break;
        }

        case EventType::WindowResized:
        {
            //const bool is_minimized = event.windowResize.value.x == 0 || event.windowResize.value.y == 0;
            //message_controller->OnWindowSizeChanged(window, event.windowResize.value.x, event.windowResize.value.y, is_minimized);
            window->on_window_resize(Vector2i(event.windowResize.value.x, event.windowResize.value.y));
            break;
        }

        case EventType::WindowMoved:
        {
            //message_controller->OnWindowMoved(window, event.windowMove.value.x, event.windowMove.value.y);
            break;
        }

        case EventType::WindowFocusGained:
        {
            //message_controller->OnWindowFocusGained();
            break;
        }

        case EventType::WindowFocusLost:
        {
            //message_controller->OnWindowFocusLost();
            break;
        }

        case EventType::KeyDown:
        {
            const bool repeat = event.key.repeat;
            const uint32_t charcode = 0;

            //message_controller->OnKeyDown(event.key.value, charcode, repeat);
            window->on_key_down(event.key.value);

            if (event.key.value == Key::Backspace)
            {
                //message_controller->OnKeyChar('\b', repeat);
            }
            else if (event.key.value == Key::Enter) 
            {
                //message_controller->OnKeyChar('\r', repeat);
            }
            break;
        }

        case EventType::KeyUp:
        {
            const uint32_t charcode = 0;
            //message_controller->OnKeyUp(event.key.value, charcode, false);
            window->on_key_up(event.key.value);
            break;
        }

        case EventType::TextInput:
        {
            const bool repeat = false;
            const ANSICHAR* UTF8 = event.text.value.buffer;

            for (const unsigned char* P = reinterpret_cast<const unsigned char*>(UTF8); P && *P; )
            {
                uint32_t Codepoint = 0;

                if ((*P & 0x80u) == 0)
                {
                    Codepoint = *P++;
                }
                else if ((*P & 0xE0u) == 0xC0u)
                {
                    Codepoint = (uint32_t)(*P++ & 0x1Fu) << 6;
                    Codepoint |= (uint32_t)(*P++ & 0x3Fu);
                }
                else if ((*P & 0xF0u) == 0xE0u)
                {
                    Codepoint = (uint32_t)(*P++ & 0x0Fu) << 12;
                    Codepoint |= (uint32_t)(*P++ & 0x3Fu) << 6;
                    Codepoint |= (uint32_t)(*P++ & 0x3Fu);
                }
                else if ((*P & 0xF8u) == 0xF0u)
                {
                    Codepoint = (uint32_t)(*P++ & 0x07u) << 18;
                    Codepoint |= (uint32_t)(*P++ & 0x3Fu) << 12;
                    Codepoint |= (uint32_t)(*P++ & 0x3Fu) << 6;
                    Codepoint |= (uint32_t)(*P++ & 0x3Fu);
                }
                else
                {
                    ++P;
                    continue;
                }

                //message_controller->OnKeyChar(Codepoint, repeat);
                break;
            }

            break;
        }

        case EventType::MouseMove:
        {
            window->on_mouse_move(Vector2(event.axis.position.x, event.axis.position.y));
            window->on_mouse_move_delta(Vector2(event.axis.delta.x, event.axis.delta.y));
            //message_controller->OnMouseMove(event.axis.position.x, event.axis.position.y);
            //message_controller->OnRawMouseMove(event.axis.delta.x, event.axis.delta.y);
            break;
        }

        case EventType::MouseButtonDown:
        {
            window->on_mouse_button_down(event.button.value);
            //message_controller->OnMouseDown(window, event.button.value);
            break;
        }

        case EventType::MouseButtonUp:
        {
            window->on_mouse_button_up(event.button.value);
            //message_controller->OnMouseUp(window, event.button.value);
            break;
        }

        case EventType::MouseWheel:
        {
            window->on_mouse_scroll(event.wheel.value);
            //message_controller->OnMouseWheel(amount);
            break;
        }

        default:
        {
            break;
        }
    }
}

MessageController *Application::GetMessageController() const
{
    return message_controller;
}

void Application::SetMessageController(MessageController* new_message_controller)
{
    message_controller = new_message_controller;
}

OSWindow* Application::InitWindow(WindowCreateInfo windowCreateInfo)
{
    std::unique_ptr<OSWindow> window = std::make_unique<OSWindow>();
    if (!window->Create(windowCreateInfo))
    {
        return nullptr;
    }

    RegisterPlatformWindow(window->WindowHandle, window->GetWindowHandleID(), &eventQueue);

    M_Windows.emplace_back(std::move(window));

    return M_Windows.back().get();
}

OSWindow* Application::FindWindowByHandle(uint64_t WindowHandleID)
{
    for (const std::unique_ptr<OSWindow>& window : M_Windows)
    {
        if (window && window->GetWindowHandleID() == WindowHandleID)
        {
            return window.get();
        }
    }

    return nullptr;
}

std::vector<const char*> Application::GetSurfaceExtensions() const
{
    uint32_t surfaceExtensionCount = 0;
    const char** surfaceExtensionData = glfwGetRequiredInstanceExtensions(&surfaceExtensionCount);

    std::vector<const char*> data;
    data.insert(data.end(), surfaceExtensionData, surfaceExtensionData + surfaceExtensionCount);

    return data;
}
