#pragma once

#include <memory>
#include <vector>

class Window;
class MessageController;

struct Event;
struct WindowCreateInfo;

class Application final
{
private:
    Application();
    ~Application() = default;

public:
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /* Access Host by singleton instance */
    static Application& Get();

    /* Initializes GLFW and platform services. Safe to call multiple times. */
    bool Initialize();

    /* Shuts down GLFW and platform services. Safe to call multiple times. */
    void Shutdown();

    /* Traverse event queue and dispatch corresponding events */
    void PumpMessageQueue();

    void ProcessDeferredMessage(const Event& event);

    void SetMessageController(MessageController* messageController);

    /* Creates a new window with set flags */
    Window* CreateWindow(const WindowCreateInfo& windowCreateInfo);

    /* Fetch window object by window handle */
    Window* FindWindowByHandle(uint64_t windowHandle);

private:
    std::vector<std::unique_ptr<Window>> M_Windows;

    MessageController* M_MessageController;

    bool M_IsValid;
};