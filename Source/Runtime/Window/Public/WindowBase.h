#pragma once

#include <cstdint>

struct RWindowInitParams
{
    const char* Title;

    int32_t Width;

    int32_t Height;
};

class IWindowBase
{
public:
    virtual ~IWindowBase() = default;

    virtual void CreateNativeWindow(RWindowInitParams& InitParams) = 0;
    virtual void DestroyNativeWindow() = 0;
    virtual void Poll() = 0;
    virtual bool ShouldClose() const = 0;
    virtual bool IsMinimized() const = 0;
    virtual bool IsFocused() const = 0;
    virtual void SetIsMinimized(bool bMinimized) = 0;
    virtual void SetIsFocused(bool bFocused) = 0;
    virtual void WaitEventOrTimeout(float TimeoutSeconds) = 0;
    virtual int32_t GetWindowWidth() const = 0;
    virtual int32_t GetWindowHeight() const = 0;
    virtual int32_t GetFramebufferWidth() const = 0;
    virtual int32_t GetFramebufferHeight() const = 0;
    virtual float GetAspectRatio() const = 0;
    virtual void* GetNativeWindow() const = 0;
};

extern IWindowBase* GWindow;