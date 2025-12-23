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
    m_MessageController = nullptr;
    m_Valid = false;
}

Application& Application::Get()
{
    static Application platform;
    return platform;
}

bool Application::Initialize()
{
    if (m_Valid)
    {
        return true;
    }

    if (glfwInit() != GLFW_TRUE)
    {
        return false;
    }

    m_Valid = true;
    return true;
}

void Application::Shutdown()
{
    if (!m_Valid)
    {
        return;
    }

    eventQueue.Clear();

    glfwTerminate();
    
    m_Windows.clear();
    m_Valid = false;
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
    std::shared_ptr<Window> window = FindWindowByHandle(event.windowHandle);
    
    switch (event.type)
    {
        case EventType::Quit:
        {
            m_MessageController->OnExit();
            break;
        }

        case EventType::WindowCloseRequested:
        {
            m_MessageController->OnWindowClose(window);
            break;
        }

        case EventType::WindowResized:
        {
            m_MessageController->OnWindowSizeChanged(window, event.windowResize.value.x, event.windowResize.value.y, false);
            break;
        }

        case EventType::WindowMoved:
        {
            m_MessageController->OnWindowMoved(window, event.windowMove.value.x, event.windowMove.value.y);
            break;
        }

        case EventType::WindowFocusGained:
        {
            m_MessageController->OnWindowFocusGained();
            break;
        }

        case EventType::WindowFocusLost:
        {
            m_MessageController->OnWindowFocusLost();
            break;
        }

        case EventType::KeyDown:
        {
            const bool repeat = event.key.repeat;
            const uint32_t charcode = 0;

            m_MessageController->OnKeyDown(event.key.value, charcode, repeat);

            if (event.key.value == Key::Backspace)
            {
                m_MessageController->OnKeyChar('\b', repeat);
            }
            else if (event.key.value == Key::Enter) 
            {
                m_MessageController->OnKeyChar('\r', repeat);
            }
            break;
        }

        case EventType::KeyUp:
        {
            const uint32_t charcode = 0;
            m_MessageController->OnKeyUp(event.key.value, charcode, false);
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

                m_MessageController->OnKeyChar(Codepoint, repeat);
                break;
            }
        }

        case EventType::MouseMove:
        {
            m_MessageController->OnRawMouseMove(event.axis.delta.x, event.axis.delta.y);
            break;
        }

        case EventType::MouseButtonDown:
        {
            m_MessageController->OnMouseDown(window, event.button.value);
            break;
        }

        case EventType::MouseButtonUp:
        {
            m_MessageController->OnMouseUp(event.button.value);
            break;
        }

        case EventType::MouseWheel:
        {
            const float amount = event.wheel.value.y * 1.0f;
            m_MessageController->OnMouseWheel(amount);
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
    m_MessageController = messageController;
}

std::shared_ptr<Window> Application::CreateWindow(const WindowCreateInfo& windowCreateInfo)
{
    std::shared_ptr<Window> window = std::make_shared<Window>();
    if (!window->Create(windowCreateInfo))
    {
        return nullptr;
    }

    m_Windows.push_back(window);

    RegisterPlatformWindow(window->nativeWindow, window->GetHandle(), &eventQueue);

    return window;
}

std::shared_ptr<Window> Application::FindWindowByHandle(uint64_t windowHandle)
{
    for (const std::shared_ptr<Window>& window : m_Windows)
    {
        if (window && window->GetHandle() == windowHandle)
        {
            return window;
        }
    }

    return nullptr;
}