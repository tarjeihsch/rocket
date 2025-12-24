#pragma once

#include <memory>
#include <vector>

class RWindow;
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

    void Upd();

    /* Creates a new window with set flags */
    RWindow* InitWindow(const WindowCreateInfo& windowCreateInfo);

    /* Fetch window object by window handle */
    RWindow* FindWindowByHandle(uint64_t WindowHandleID);

    std::vector<const char*> GetSurfaceExtensions() const;

private:
    std::vector<std::unique_ptr<RWindow>> M_Windows;

    MessageController* M_MessageController;

    bool M_IsValid;
};
