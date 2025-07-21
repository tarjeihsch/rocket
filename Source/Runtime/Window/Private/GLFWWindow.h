#pragma once

#include "Window.h"

#if defined(_WIN32)
#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

class CGLFWWindow : public IWindow
{
public:
	CGLFWWindow(const SWindowSpecification& InWindowSpecification)
		: IWindow(InWindowSpecification)
	{
	}

	virtual void CreateNativeWindow() override;
	virtual void DestroyNativeWindow() override;
	virtual void Poll() override;
	virtual bool ShouldClose() const override;
	virtual bool IsMinimized() const override;
	virtual bool IsFocused() const override;
	virtual void SetIsMinimized(bool bMinimized) override;
	virtual void SetIsFocused(bool bFocused) override;
	virtual void SetFocus(bool bFocus);
	virtual void WaitEventOrTimeout(float TimeoutSeconds) override;
};
