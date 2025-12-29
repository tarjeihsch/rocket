#pragma once

#include <string>
#include <vector>
#include <cstdint>

class Viewport;
struct FRHIBuffer;
struct RHITexture;
class ViewportDesc;
class FrameGraph;

struct PendingBufferUpload
{
	std::string DebugName;
	FRHIBuffer* Dst = nullptr;
	FRHIBuffer* Staging = nullptr;
};

struct PendingTextureUpload
{
	std::string DebugName;
	RHITexture* Dst = nullptr;
	FRHIBuffer* Staging = nullptr;
};

struct ClearColor
{
	float r, g, b, a;
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	void shutdown();

	static Renderer& get();

	void begin_frame();
	void end_frame();

	void clear_color(ClearColor clear_color);

private:
	std::vector<PendingBufferUpload> m_pending_buffer_uploads;
	std::vector<PendingTextureUpload> m_pending_texture_uploads;

	Viewport* m_viewport;
	FrameGraph* m_frame_graph;
	
	uint8_t m_viewport_flag : 1;
	uint8_t m_clear_color_flag : 1;
};