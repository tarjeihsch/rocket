#pragma once

#include "Math/Vector2.h"

#include <memory>

class RRHIViewport;
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
	RRHIViewport* Viewport;
	RWindow* Window;
};