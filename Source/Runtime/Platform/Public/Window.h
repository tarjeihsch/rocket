#pragma once

#include "CoreTypes.h"
#include "PlatformTypes.h"
#include "Event.h"

struct WindowCreateInfo
{
    const ANSICHAR* title;
    int32_t width;
    int32_t height;
    bool resizable;
    bool visible;
    bool hdpi;
};

class Window
{
public:
    Window();
    ~Window();
        
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool Create(const WindowCreateInfo& createInfo);
    void Destroy();

    void Show();
    void Hide();

    void SetTitle(const ANSICHAR* title);

    Vector2i GetSize() const;
    Vector2i GetPosition() const;

    uint64_t GetHandle() const;
    bool IsValid() const;

private:
    void* m_NativeWindow;
    uint64_t windowHandle;

    friend class Application;
};
