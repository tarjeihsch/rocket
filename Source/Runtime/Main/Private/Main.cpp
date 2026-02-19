#include "Application.h"
#include "Window.h"
#include "MessageController.h"
#include "Common/RHI.h"
#include "Utils/File.h"
#include "Utils/Time.h"
bool GWindowShouldClose = false;

RHI::GPUSwapchain* swapchain = nullptr;
RHI::GPUTexture* texture = nullptr;

static bool window_minimized = false;
static bool window_size_changed = false;
static bool window_should_close = false;

class EngineMessageController : public MessageController
{
	virtual bool OnWindowSizeChanged(OSWindow* window, int32_t width, int32_t height, bool bWasMinimized)
	{
		window_minimized = width == 0 || height == 0;
		window_size_changed = true;
		
		return true;
	}

	void OnWindowClose(OSWindow* Window) override
	{
		GWindowShouldClose = true;
	}

	virtual bool OnKeyDown(Key key, uint32_t CharCode, bool bIsRepeat)
	{
		if (key == Key::Escape)
		{
			GWindowShouldClose = true;
		}
		return true;
	}
};

static EngineMessageController GEngineMessageController;

int main()
{
	Application::Get().Initialize();
	Application::Get().SetMessageController(&GEngineMessageController);

	RHI::Initialize(RHI::GPUDriver::Vulkan);

	std::vector<char> vertex_shader_bytecode = LoadFile(ROCKET_BINARIES "/Shaders/Shader.vert.spv");
	std::vector<char> fragment_shader_bytecode = LoadFile(ROCKET_BINARIES "/Shaders/Shader.frag.spv");

	RHI::GPUShaderDesc vertex_shader_desc;
	vertex_shader_desc.bytecode = vertex_shader_bytecode.data();
	vertex_shader_desc.bytecode_size = vertex_shader_bytecode.size();
	vertex_shader_desc.stage_flag = RHI::GPUShaderStageFlag::Vertex;
	RHI::GPUShader* vertex_shader = RHI::GetRHI()->CreateShader(vertex_shader_desc);

	RHI::GPUShaderDesc fragment_shader_desc;
	fragment_shader_desc.bytecode = fragment_shader_bytecode.data();
	fragment_shader_desc.bytecode_size = fragment_shader_bytecode.size();
	fragment_shader_desc.stage_flag = RHI::GPUShaderStageFlag::Fragment;
	RHI::GPUShader* fragment_shader = RHI::GetRHI()->CreateShader(fragment_shader_desc);

	RHI::GPUFormat color_formats[] = { RHI::GPUFormat::BGRA8_UNORM };
	RHI::GPUGraphicsPipelineDesc graphics_pipeline_desc;
	graphics_pipeline_desc.vertex_shader = vertex_shader;
	graphics_pipeline_desc.fragment_shader = fragment_shader;
	graphics_pipeline_desc.topology = RHI::GPUPrimitiveTopology::TriangleList;
	graphics_pipeline_desc.cull_mode = RHI::GPUCullMode::Back;
	graphics_pipeline_desc.front_face = RHI::GPUFrontFace::CounterClockwise;
	graphics_pipeline_desc.polygon_mode = RHI::GPUPolygonMode::Fill;
	graphics_pipeline_desc.color_formats = color_formats;
	graphics_pipeline_desc.color_format_count = 1;
	graphics_pipeline_desc.depth_format = RHI::GPUFormat::D32_FLOAT;
	graphics_pipeline_desc.depth_compare_op = RHI::GPUCompareOp::Less;
	graphics_pipeline_desc.depth_test_enable = true;
	graphics_pipeline_desc.depth_write_enable = true;
	graphics_pipeline_desc.blend_enable = false;
	RHI::GPUPipeline* graphics_pipeline = RHI::GetRHI()->CreateGraphicsPipeline(graphics_pipeline_desc);
		
	WindowCreateInfo window_create_info = {};
	window_create_info.title = "Rocket Engine";
	window_create_info.width = 1280;
	window_create_info.height = 720;
	window_create_info.resizable = true;
	window_create_info.visible = true;
	window_create_info.hdpi = true;
	OSWindow* window = Application::Get().InitWindow(window_create_info);

	RHI::GPUSwapchainDesc swapchain_desc;
	swapchain_desc.backbuffer_count = 3;
	swapchain_desc.backbuffer_format = RHI::GPUFormat::BGRA8_SRGB;
	swapchain_desc.backbuffer_width = 1280;
	swapchain_desc.backbuffer_height = 720;
	swapchain_desc.present_mode = RHI::GPUPresentMode::Mailbox;
	swapchain_desc.window_platform.hwnd = static_cast<HWND>(window->GetNativeWindowHandle());
	swapchain = RHI::GetRHI()->CreateSwapchain(swapchain_desc);

	RHI::GPUBufferDesc buffer_desc = RHI::GPUBufferDesc::StorageBuffer(4ULL * 1024 * 1024);
	RHI::GPUBuffer* buffer = RHI::GetRHI()->CreateBuffer(buffer_desc);

	RHI::GPUTextureDesc texture_desc = RHI::GPUTextureDesc::RenderTarget(RHI::GPUFormat::BGRA8_SRGB, swapchain->GetBackBufferSize().x, swapchain->GetBackBufferSize().y);
	texture = RHI::GetRHI()->CreateTexture(texture_desc);

	RHI::GPUCommandListDesc command_list_desc;
	command_list_desc.queue_flags = RHI::GPUQueueFlag::Graphics;
	RHI::GPUCommandList* command_list = RHI::GetRHI()->CreateCommandList(command_list_desc);

	RHI::GPUTextureBarrier barriers[4];

	uint64_t Last_Frame_Time = Stopwatch::GetTicks();

	while (!GWindowShouldClose)
	{
			uint64_t New_Frame_Time = Stopwatch::GetTicks();
			Timespan Delta_Frame_Time = Stopwatch::GetDelta(Last_Frame_Time, New_Frame_Time);
			float Delta_Time = static_cast<float>(Delta_Frame_Time.ToSeconds());
			Last_Frame_Time = New_Frame_Time;

		Application::Get().PumpMessageQueue();

		if (window_minimized)
		{
			continue;
		}

		if (window_size_changed)
		{
			swapchain->ResizeBackBuffers();
			delete texture;
			RHI::GPUTextureDesc texture_desc = RHI::GPUTextureDesc::RenderTarget(RHI::GPUFormat::BGRA8_SRGB, swapchain->GetBackBufferSize().x, swapchain->GetBackBufferSize().y);
			texture = RHI::GetRHI()->CreateTexture(texture_desc);
			window_size_changed = false;
		}

		swapchain->AcquireNextBackBuffer();
		
		auto timing_query_result = RHI::GetRHI()->GetTimingQueryResult();
		auto pipeline_query_result = RHI::GetRHI()->GetPipelineQueryResult();

		command_list->Begin();
		command_list->BeginTimelineQuery();
		command_list->BeginPipelineQuery();

		command_list->SetPipeline(graphics_pipeline);

		barriers[0].texture = texture;
		barriers[0].old_layout = RHI::GPUTextureLayout::Undefined;
		barriers[0].new_layout = RHI::GPUTextureLayout::ColorAttachment;
		command_list->Barrier(nullptr, 0, barriers, 1);

		RHI::GPUColorAttachment color_attachment;
		color_attachment.M_Clear_Value.color.r = 0;
		color_attachment.M_Clear_Value.color.g = 1;
		color_attachment.M_Clear_Value.color.b = 0;
		color_attachment.M_Clear_Value.color.a = 1;
		color_attachment.M_Load_Op = RHI::GPULoadOp::Clear;
		color_attachment.M_Store_Op = RHI::GPUStoreOp::Store;
		color_attachment.M_Texture = texture;

		RHI::GPURenderPass render_pass;
		render_pass.color_attachments[0] = &color_attachment;
		render_pass.color_attachment_count = 1;
		render_pass.render_width = swapchain->GetBackBufferSize().x;
		render_pass.render_height = swapchain->GetBackBufferSize().y;
		command_list->BeginRenderPass(&render_pass);

		command_list->EndRenderPass();

		barriers[0].texture = texture;
		barriers[0].old_layout = RHI::GPUTextureLayout::ColorAttachment;
		barriers[0].new_layout = RHI::GPUTextureLayout::TransferSrc;
		barriers[1].texture = swapchain->GetBackBufferTexture();
		barriers[1].old_layout = RHI::GPUTextureLayout::Undefined;
		barriers[1].new_layout = RHI::GPUTextureLayout::TransferDst;
		command_list->Barrier(nullptr, 0, barriers, 2);

		command_list->CopyTextureToTexture(texture, swapchain->GetBackBufferTexture(), swapchain->GetBackBufferSize().x, swapchain->GetBackBufferSize().y);

		barriers[0].texture = swapchain->GetBackBufferTexture();
		barriers[0].old_layout = RHI::GPUTextureLayout::TransferDst;
		barriers[0].new_layout = RHI::GPUTextureLayout::Present;
		command_list->Barrier(nullptr, 0, barriers, 1);
		
		command_list->EndTimelineQuery();
		command_list->EndPipelineQuery();
		command_list->End();

		RHI::GetRHI()->Submit(command_list);

		swapchain->PresentBackBuffer();
	}
	
	RHI::GetRHI()->WaitIdle();
	
	delete buffer;
	delete texture;
	delete vertex_shader;
	delete fragment_shader;
	delete graphics_pipeline;
	delete swapchain;
	delete command_list;

	RHI::Shutdown();

	Application::Get().Shutdown();
}