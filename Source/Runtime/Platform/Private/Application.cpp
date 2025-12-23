#include "Application.h"

#include "Assertion.h"
#include "CoreTypes.h"
#include "Event.h"
#include "Window.h"
#include "MessageController.h"
#include "EventSink.h"

#include <mutex>
#include <deque>
#include <GLFW/glfw3.h>

void RegisterPlatformWindow(void* nativeWindow, uint64_t windowHandle, EventSink* eventSink);

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
    M_MessageController = nullptr;
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
        ProcessDeferredMessage(event);
    }
}

void Application::ProcessDeferredMessage(const Event& event)
{
    Platform::Window* window = FindWindowByHandle(event.windowHandle);
    
    switch (event.type)
    {
        case EventType::Quit:
        {
            M_MessageController->OnExit();
            break;
        }

        case EventType::WindowCloseRequested:
        {
            M_MessageController->OnWindowClose(window);
            break;
        }

        case EventType::WindowResized:
        {
            M_MessageController->OnWindowSizeChanged(window, event.windowResize.value.x, event.windowResize.value.y, false);
            break;
        }

        case EventType::WindowMoved:
        {
            M_MessageController->OnWindowMoved(window, event.windowMove.value.x, event.windowMove.value.y);
            break;
        }

        case EventType::WindowFocusGained:
        {
            M_MessageController->OnWindowFocusGained();
            break;
        }

        case EventType::WindowFocusLost:
        {
            M_MessageController->OnWindowFocusLost();
            break;
        }

        case EventType::KeyDown:
        {
            const bool repeat = event.key.repeat;
            const uint32_t charcode = 0;

            M_MessageController->OnKeyDown(event.key.value, charcode, repeat);

            if (event.key.value == Key::Backspace)
            {
                M_MessageController->OnKeyChar('\b', repeat);
            }
            else if (event.key.value == Key::Enter) 
            {
                M_MessageController->OnKeyChar('\r', repeat);
            }
            break;
        }

        case EventType::KeyUp:
        {
            const uint32_t charcode = 0;
            M_MessageController->OnKeyUp(event.key.value, charcode, false);
            break;
        }

        case EventType::TextInput:
        {
            const bool repeat = false;
            const ANSICHAR* UTF8 = event.text.value.buffer;

            // UTF-8 decode loop with no allocations.
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
                    // Invalid sequence; skip.
                    ++P;
                    continue;
                }

                M_MessageController->OnKeyChar(Codepoint, repeat);
                break;
            }
        }

        case EventType::MouseMove:
        {
            M_MessageController->OnRawMouseMove(event.axis.delta.x, event.axis.delta.y);
            break;
        }

        case EventType::MouseButtonDown:
        {
            M_MessageController->OnMouseDown(window, event.button.value);
            break;
        }

        case EventType::MouseButtonUp:
        {
            M_MessageController->OnMouseUp(event.button.value);
            break;
        }

        case EventType::MouseWheel:
        {
            const float amount = event.wheel.value.y * 1.0f;
            M_MessageController->OnMouseWheel(amount);
            break;
        }

        default:
        {
            break;
        }
    }
}

void Application::SetMessageController(MessageController* messageController)
{
    M_MessageController = messageController;
}

Platform::Window* Application::CreateWindow(const WindowCreateInfo& windowCreateInfo)
{
    std::unique_ptr<Platform::Window> window = std::make_unique<Platform::Window>();
    if (!window->Create(windowCreateInfo))
    {
        return nullptr;
    }

    RegisterPlatformWindow(window->m_NativeWindow, window->GetHandle(), &eventQueue);

    M_Windows.emplace_back(std::move(window));

    return M_Windows.back().get();
}

Platform::Window* Application::FindWindowByHandle(uint64_t windowHandle)
{
    for (const std::unique_ptr<Platform::Window>& window : M_Windows)
    {
        if (window && window->GetHandle() == windowHandle)
        {
            return window.get();
        }
    }

    return nullptr;
}

Platform::Window* Application::FindWindowByIndex(uint32_t index)
{
    if (M_Windows.size() >= index)
    {
        return M_Windows[index].get();
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
