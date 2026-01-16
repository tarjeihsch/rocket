#include "Viewport.h"

#include "Application.h"
#include "Window.h"
#include "Common/RHI.h"
#include "Common/RHISurface.h"

Viewport::Viewport()
{
	Init();
}

Viewport::~Viewport()
{
	Shutdown();
}

void Viewport::Init()
{
	WindowCreateInfo window_create_info = {};
	window_create_info.title = "Rocket Engine";
	window_create_info.width = 1280;
	window_create_info.height = 720;
	window_create_info.visible = true;
	window_create_info.hdpi = true;
	window_create_info.resizable = true;
	window = Application::Get().InitWindow(window_create_info);

	RHISurfaceDesc surface_desc = {};
	surface_desc.Backend = window->GetNativeBackend();
	surface_desc.NativeWindowHandle = window->GetNativeWindowHandle();
	surface_desc.NativeInstanceHandle = window->GetNativeWindowSystemHandle();
	surface = GetRHI()->CreateSurface(surface_desc);
}

void Viewport::Shutdown()
{
	delete surface;
}