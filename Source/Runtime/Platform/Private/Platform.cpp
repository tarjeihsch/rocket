#include "Platform.h"

#include "Application.h"
#include "Window.h"

namespace Platform
{
    std::vector<const char*> GetSurfaceExtensions()
    {
        return Application::Get().GetSurfaceExtensions();        
    }

    Platform::Window* GetWindowFromIndex(uint32_t index)
    {
        return Application::Get().FindWindowByIndex(index);
    }

    Vector2f GetWindowExtent(const Window* window)
    {
        return window->GetSize();
    }
}
