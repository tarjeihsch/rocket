#pragma once

#include <cstdint>
#include <vector>

#include "Application.h"
#include "Math/Vector2.h"

namespace Platform
{
    class Window;

    std::vector<const char*> GetSurfaceExtensions();

    Platform::Window* GetWindowFromIndex(uint32_t index);

    Vector2f GetWindowExtent(const Window* window);
    Vector2f GetFramebufferExtent(const Window* window);
}
