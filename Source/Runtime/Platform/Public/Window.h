#pragma once

#include "CoreTypes.h"
#include "Delegate.h"
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

using OnWindowResize = Delegate<void(Vector2i)>;
using OnWindowResizeMulticast = MulticastDelegate<void(Vector2i)>;
DECLARE_DELEGATE_OneParam(OnMouseMove, void, Vector2);
DECLARE_DELEGATE_OneParam(OnMouseMoveDelta, void, Vector2);
DECLARE_DELEGATE_OneParam(OnMouseScroll, void, Vector2);
DECLARE_DELEGATE_TwoParams(OnMouseButtonDown, void, Button, bool);
DECLARE_DELEGATE_OneParam(OnMouseButtonUp, void, Button);
DECLARE_DELEGATE_TwoParams(OnKeyDown, void, Key, bool);
DECLARE_DELEGATE_OneParam(OnKeyUp, void, Key);
DECLARE_DELEGATE_OneParam(OnTextInput, void, const ANSICHAR*);
DECLARE_DELEGATE(OnWindowClose, void);

class OSWindow
{
public:
    /* Both are called with the new size of the window when it is resized, the single delegate first */
    OnWindowResize on_window_resize;
    OnWindowResizeMulticast on_window_resize_multicast;
    OnMouseMove on_mouse_move;
    OnMouseMoveDelta on_mouse_move_delta;
    OnMouseScroll on_mouse_scroll;
    OnMouseButtonDown on_mouse_button_down;
    OnMouseButtonUp on_mouse_button_up;
    OnKeyDown on_key_down;
    OnKeyUp on_key_up;
    OnTextInput on_text_input;
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

    /* Size in screen coordinates, which is not the size in pixels on a scaled display */
    Vector2i GetSize() const;

    /* Size of the drawable area in pixels, which is what a swapchain is sized to. Zero when minimized */
    Vector2i GetFramebufferSize() const;

    Vector2i GetPosition() const;

    uint64_t GetWindowHandleID() const;
    ENativePlatformBackend GetNativeBackend() const;
    RNativeWindowSystemHandle GetNativeWindowSystemHandle() const;
    RNativeWindowHandle GetNativeWindowHandle() const;

    bool IsValid() const;
    bool ShouldClose() const;
    void RequestClose();
    bool ShouldResize() const;
    void RequestResize();
    void ClearResize();
    bool IsFocused() const;

private:
    void* WindowHandle;
    uint64_t WindowHandleID;
    bool ResizeRequested;

    ENativePlatformBackend NativeBackend;
    RNativeWindowSystemHandle NativeWindowSystemHandle = nullptr;
    RNativeWindowHandle NativeWindowHandle = nullptr;

    friend class Application;
};
