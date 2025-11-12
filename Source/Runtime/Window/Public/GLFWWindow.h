#pragma once

#include "Delegate.h"
#include "WindowBase.h"

#include <cstdint>

#ifdef RK_PLATFORM_WINDOWS
#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

class RGLFWWindow final : public IWindowBase
{
public:
    TDelegate<uint32_t> OnWindowFocusDelegate;

    RGLFWWindow();

    virtual void CreateNativeWindow(RWindowInitParams& InitParams) override;
    virtual void DestroyNativeWindow() override;
    virtual void Poll() override;
    virtual bool ShouldClose() const override;
    virtual bool IsMinimized() const override;
    virtual bool IsFocused() const override;
    virtual void SetIsMinimized(bool bMinimized) override;
    virtual void SetIsFocused(bool bFocused) override;
    virtual int32_t GetWindowWidth() const override;
    virtual int32_t GetWindowHeight() const override;
    virtual int32_t GetFramebufferWidth() const override;
    virtual int32_t GetFramebufferHeight() const override;
    virtual void WaitEventOrTimeout(float TimeoutSeconds) override;
    virtual float GetAspectRatio() const override;
    virtual void* GetNativeWindow() const override;

private:
    void* NativeWindow;

    uint8_t bIsMinimized : 1;

    uint8_t bIsFocused : 1;
};
