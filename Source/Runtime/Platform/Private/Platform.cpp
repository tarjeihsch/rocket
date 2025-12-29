#include "Platform.h"

#include "Application.h"

namespace Platform
{
    std::vector<const char*> GetSurfaceExtensions()
    {
        return Application::get().GetSurfaceExtensions();        
    }
}
