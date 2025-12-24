#pragma once

#include "Math/Vector2.h"

#include <memory>

class RRHISwapchain;
class RWindow;

struct RViewport
{
	Vector2i Extent;
	Vector2i Scissor;
};

class RDisplay
{
public:
	RDisplay();
	~RDisplay();

	void Init();
	void Shutdown();

	void Present();

private:
	RRHISwapchain* Swapchain;
	RWindow* Window;
};