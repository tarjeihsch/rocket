#pragma once

#include "Math/Vector2.h"

#include <memory>

struct RHITexture;
class RHISurface;
class OSWindow;

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

	inline OSWindow* GetWindow() const;
	inline RHISurface* GetSurface() const;

private:
	void Init();
	void Shutdown();

private:
	OSWindow* window;
	RHISurface* surface;
};

inline OSWindow* Viewport::GetWindow() const
{
	return window;
}

RHISurface* Viewport::GetSurface() const
{
	return surface;
}