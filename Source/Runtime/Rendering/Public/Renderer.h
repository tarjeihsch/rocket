#pragma once

#include <cstdint>

class RHISurface;
class RenderGraph;
class Viewport;
class RenderTarget;

class Renderer
{
public:
	struct Context
	{
		RHISurface* target_surface;

		uint32_t viewport_width;
		uint32_t viewport_height;
	};

	enum State
	{
		ST_IsFinished,
		ST_HasBegun,
	};

	static Renderer& Get();

	void Initialize();
	void Shutdown();

	void BeginFrame(Viewport* viewport);
	void EndFrame();

private:
	RenderGraph* render_graph;

	Context context;

	State state;
};

#define RENDER_COMMAND_GRAPHICS_PASS(x)