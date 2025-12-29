#pragma once

#include "Math/Vector2.h"

#include <memory>

struct RHITexture;
class RHISurface;
class Window;

struct ViewportDesc
{
	Vector2i Extent;
	Vector2i Scissor;
};

class Viewport
{
public:
	Viewport();
	~Viewport();

	inline Window* get_window() const;
	inline RHISurface* get_surface() const;

private:
	void init();
	void shutdown();

private:
	Window* window;

	RHISurface* surface;
};

inline Window* Viewport::get_window() const
{
	return window;
}

RHISurface* Viewport::get_surface() const
{
	return surface;
}