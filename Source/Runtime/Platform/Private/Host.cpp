#include "Host.h"

#include "CoreTypes.h"
#include "PlatformEvent.h"
#include "PlatformEventQueue.h"
#include "PlatformWindow.h"

#include <mutex>
#include <deque>
#include <GLFW/glfw3.h>

namespace Platform 
{
    void RegisterPlatformWindow(void* nativeWindow, uint64_t windowHandle, EventSink* eventSink);

    class EventQueue : public EventSink
    {
    public:
        void Enqueue(const Event& event) override
        {
            std::scoped_lock lock(mutex);
            queue.push_back(event);
        }

        bool Dequeue(Event& event) override
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

    Host::Host()
        : messageHandler(nullptr),
        isInitialized(false)
    {
    }

    Host& Host::Get()
    {
        static Host platform;
        return platform;
    }

    bool Host::Initialize()
    {
        if (isInitialized)
        {
            return true;
        }

        if (glfwInit() != GLFW_TRUE)
        {
            return false;
        }

        isInitialized = true;
        return true;
    }

    void Host::Shutdown()
    {
        if (!isInitialized)
        {
            return;
        }

        eventQueue.Clear();

        windows.clear();
        messageHandler = nullptr;

        glfwTerminate();
        isInitialized = false;
    }

    void Host::PumpOSMessageQueue(float deltaTime)
    {
        glfwPollEvents();

        Event event = {};
        while (eventQueue.Dequeue(event))
        {
            DispatchEvent(event);
        }
    }

    std::shared_ptr<Window> Host::CreateWindow(const WindowCreateInfo& windowCreateInfo)
    {
        std::shared_ptr<Window> window = std::make_shared<Window>();
        if (!window->Create(windowCreateInfo))
        {
            return nullptr;
        }

        windows.push_back(window);

        RegisterPlatformWindow(window->nativeWindow, window->GetHandle(), &eventQueue);

        return window;
    }

    std::shared_ptr<Window> Host::FindWindowByHandle(uint64_t windowHandle)
    {
        for (const std::shared_ptr<Window>& window : windows)
        {
            if (window && window->GetHandle() == windowHandle)
            {
                return window;
            }
        }

        return nullptr;
    }

    void Host::SetMessageHandler(HostMessageHandler* newMessageHandler)
    {
        messageHandler = newMessageHandler;
    }

    void Host::DispatchEvent(const Event& event)
    {
        if (messageHandler)
        {
            messageHandler->OnEvent(event);
        }
    }
}
