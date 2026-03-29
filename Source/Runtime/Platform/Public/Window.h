#pragma once

#include "CoreTypes.h"
#include "PlatformTypes.h"
#include "Event.h"
#include "Math/Vector.h"

struct WindowCreateInfo
{
    const ANSICHAR* title;
    int32_t width;
    int32_t height;
    bool resizable;
    bool visible;
    bool hdpi;
};

class OSWindow
{
public:
    OSWindow();
    ~OSWindow();

    OSWindow(const OSWindow&) = delete;
    OSWindow& operator=(const OSWindow&) = delete;

    bool Create(const WindowCreateInfo& createInfo);
    void Destroy();

    void Show();
    void Hide();

    void SetTitle(const ANSICHAR* title);

    void WaitEvents();

    Vector2i GetSize() const;
    Vector2i GetPosition() const;

    uint64_t GetWindowHandleID() const;
    ENativePlatformBackend GetNativeBackend() const;
    RNativeWindowSystemHandle GetNativeWindowSystemHandle() const;
    RNativeWindowHandle GetNativeWindowHandle() const;

    bool IsValid() const;
    bool ShouldClose() const;

private:
    void* WindowHandle;
    uint64_t WindowHandleID;
        
    ENativePlatformBackend NativeBackend;
    RNativeWindowSystemHandle NativeWindowSystemHandle = nullptr;
    RNativeWindowHandle NativeWindowHandle = nullptr;

    friend class Application;
};
