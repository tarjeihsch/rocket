#include "Viewport.h"

#include "Application.h"
#include "Window.h"
#include "Common/RHI.h"
#include "Common/RHISurface.h"

Viewport::Viewport()
{
	init();
}

Viewport::~Viewport()
{
	shutdown();
}

void Viewport::init()
{
	WindowCreateInfo window_create_info = {};
	window_create_info.title = "Rocket Engine";
	window_create_info.width = 1280;
	window_create_info.height = 720;
	window_create_info.visible = true;
	window_create_info.hdpi = true;
	window_create_info.resizable = true;
	window = Application::get().init_window(window_create_info);

	RHISurfaceDesc surface_desc = {};
	surface_desc.Backend = window->GetNativeBackend();
	surface_desc.NativeWindowHandle = window->GetNativeWindowHandle();
	surface_desc.NativeInstanceHandle = window->GetNativeWindowSystemHandle();
	surface = GetRHI()->create_surface(surface_desc);
}

void Viewport::shutdown()
{
	delete surface;
}