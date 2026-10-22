#pragma once

#include <memory>
#include <vector>
#include <cstdint>

class OSWindow;
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

    /**
     * Runs an infinite loop (or until timeout) while waiting for a new event.
     */
    void WaitForEvents(double timeout = DBL_MAX);

    void ProcessDeferredEvent(const Event& event);

    MessageController* GetMessageController() const;

    void SetMessageController(MessageController* messageController);

    /* Creates a new window with set flags */
    OSWindow* InitWindow(WindowCreateInfo windowCreateInfo);

    /* Fetch window object by window handle */
    OSWindow* FindWindowByHandle(uint64_t WindowHandleID);

    /* Window created first, used as the game's main window */
    OSWindow* GetMainWindow() const;

    std::vector<const char*> GetSurfaceExtensions() const;

private:
    std::vector<std::unique_ptr<OSWindow>> M_Windows;

    bool M_IsValid;
};
