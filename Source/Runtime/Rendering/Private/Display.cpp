#include "Display.h"

#include "Window.h"
#include "Common/RHI.h"
#include "Common/RHISwapchain.h"

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

	RRHISwapchainDesc SwapchainDesc = {};
	SwapchainDesc.Backend = Window->GetNativeBackend();
	SwapchainDesc.NativeWindowHandle = Window->GetNativeWindowHandle();
	SwapchainDesc.NativeInstanceHandle = Window->GetNativeWindowSystemHandle();

	Swapchain = GetRHI()->CreateSwapchain(SwapchainDesc);
}

void RDisplay::Shutdown()
{
}

void RDisplay::Present()
{
	// Get backbuffer from swapchain -> blit and present
}
