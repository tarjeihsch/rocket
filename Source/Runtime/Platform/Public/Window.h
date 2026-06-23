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

DECLARE_DELEGATE_OneParam(OnWindowResize, void, Vector2i);
DECLARE_DELEGATE_OneParam(OnMouseMove, void, Vector2);
DECLARE_DELEGATE_OneParam(OnMouseMoveDelta, void, Vector2);
DECLARE_DELEGATE_OneParam(OnMouseButtonDown, void, Button);
DECLARE_DELEGATE_OneParam(OnMouseButtonUp, void, Button);
DECLARE_DELEGATE_OneParam(OnKeyDown, void, Key);
DECLARE_DELEGATE_OneParam(OnKeyUp, void, Key);
DECLARE_DELEGATE(OnWindowClose, void);

class OSWindow
{
public:
    OnWindowResize on_window_resize;
    OnMouseMove on_mouse_move;
    OnMouseMoveDelta on_mouse_move_delta;
    OnMouseButtonDown on_mouse_button_down;
    OnMouseButtonUp on_mouse_button_up;
    OnKeyDown on_key_down;
    OnKeyUp on_key_up;
    OnWindowClose on_window_close;

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
    bool IsFocused() const;

private:
    void* WindowHandle;
    uint64_t WindowHandleID;

    ENativePlatformBackend NativeBackend;
    RNativeWindowSystemHandle NativeWindowSystemHandle = nullptr;
    RNativeWindowHandle NativeWindowHandle = nullptr;

    friend class Application;
};
