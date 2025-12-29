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
    static Application& get();

    /* Initializes GLFW and platform services. Safe to call multiple times. */
    bool initialize();

    /* Shuts down GLFW and platform services. Safe to call multiple times. */
    void shutdown();

    /* Traverse event queue and dispatch corresponding events */
    void pump_message_queue();

    void process_deferred_message(const Event& event);

    void set_message_controller(MessageController* messageController);

    /* Creates a new window with set flags */
    Window* init_window(WindowCreateInfo windowCreateInfo);

    /* Fetch window object by window handle */
    Window* FindWindowByHandle(uint64_t WindowHandleID);

    std::vector<const char*> GetSurfaceExtensions() const;

private:
    std::vector<std::unique_ptr<Window>> M_Windows;

    MessageController* M_MessageController;

    bool M_IsValid;
};
