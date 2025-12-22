#pragma once

#include <memory>
#include <vector>

namespace Platform
{
    class Window;
    struct Event;
    struct WindowCreateInfo;

    class HostMessageHandler
    {
    public:
        virtual ~HostMessageHandler() = default;

        virtual void OnEvent(const Event& event) = 0;
    };

    class Host final
    {
    private:
        Host();
        ~Host() = default;

    public:
        Host(const Host&) = delete;
        Host& operator=(const Host&) = delete;

        /* Access Host by singleton instance */
        static Host& Get();

        /* Initializes SDL3 and platform services. Safe to call multiple times. */
        bool Initialize();

        /* Shuts down SDL3 and platform services. Safe to call multiple times. */
        void Shutdown();

        /* Traverse event queue and dispatch corresponding events */
        void PumpOSMessageQueue(float deltaTime);

        /* Creates a new window with set flags */
        std::shared_ptr<Window> CreateWindow(const WindowCreateInfo& windowCreateInfo);

        /* Fetch window object by window handle */
        std::shared_ptr<Window> FindWindowByHandle(uint64_t windowHandle);

        /* Binds message callback handler */
        void SetMessageHandler(HostMessageHandler* messageHandler);

    protected:
        void DispatchEvent(const Event& event);

    private:
        std::vector<std::shared_ptr<Window>> windows;

        HostMessageHandler* messageHandler;

        bool isInitialized;
    };
}
