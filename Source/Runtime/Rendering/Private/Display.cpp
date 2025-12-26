#include "Display.h"

#include "Window.h"
#include "Common/RHI.h"
#include "Common/RHISwapchain.h"
#include "Common/RHIViewport.h"

RDisplay::RDisplay()
{
	Init();
}

RDisplay::~RDisplay()
{
	Shutdown();
}

void RDisplay::Init()
{
	SetRHIModule(ERHIInterfaceType::Vulkan);
	GRHI->Init();

	WindowCreateInfo windowCreateInfo = {};
	windowCreateInfo.title = "Rocket Engine";
	windowCreateInfo.width = 1280;
	windowCreateInfo.height = 720;
	windowCreateInfo.visible = true;
	windowCreateInfo.hdpi = true;
	windowCreateInfo.resizable = true;

	Window = new RWindow();
	Window->Create(windowCreateInfo);

	RRHIViewportDesc ViewportDesc = {};
	ViewportDesc.Backend = Window->GetNativeBackend();
	ViewportDesc.NativeWindowHandle = Window->GetNativeWindowHandle();
	ViewportDesc.NativeInstanceHandle = Window->GetNativeWindowSystemHandle();

	Viewport = GetRHI()->CreateViewport(ViewportDesc);
}

void RDisplay::Shutdown()
{
}

void RDisplay::Present()
{
	// Get backbuffer from swapchain -> blit and present
}
