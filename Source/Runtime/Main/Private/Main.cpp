#include "Application.h"
#include "Window.h"
#include "MessageController.h"
#include "Common/RHI.h"
#include "Utils/File.h"

bool GWindowShouldClose = false;

class EngineMessageController : public MessageController
{
	void OnWindowClose(OSWindow* Window) override
	{
		GWindowShouldClose = true;
	}
};

static EngineMessageController GEngineMessageController;

int main()
{
	Application::Get().Initialize();
	Application::Get().SetMessageController(&GEngineMessageController);

	RHI::Initialize(RHI::GPUDriver::Vulkan);

	RHI::GPUBufferDesc buffer_desc = RHI::GPUBufferDesc::StorageBuffer(4ULL * 1024 * 1024);
	RHI::GPUBuffer* buffer = RHI::GetRHI()->CreateBuffer(buffer_desc);
	
	RHI::GPUTextureDesc texture_desc = RHI::GPUTextureDesc::Texture2D(RHI::GPUFormat::RGBA32_FLOAT, 1280, 720);
	RHI::GPUTexture* texture = RHI::GetRHI()->CreateTexture(texture_desc);

	RHI::GPUBindGroupEntry srv_array;
	srv_array.position = 0;
	srv_array.count = 128;
	srv_array.bind_type = RHI::GPUBindGroupEntryType::SampledTexture;

	RHI::GPUBindGroupEntry uav_array;
	uav_array.position = 1;
	uav_array.count = 128;
	uav_array.bind_type = RHI::GPUBindGroupEntryType::StorageTexture;

	RHI::GPUBindGroupEntry entries[2] = { srv_array, uav_array };

	RHI::GPUBindGroupDesc bind_group_desc;
	bind_group_desc.entries = entries;
	bind_group_desc.entry_count = sizeof(entries) / sizeof(entries[0]);

	RHI::GPUShaderResources* bind_group = RHI::GetRHI()->CreateBindGroup(bind_group_desc);
	bind_group->SetTexture(texture, 0, 0);

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
	RHI::GPUShaderResources* bind_groups[] = { bind_group };

	RHI::GPUGraphicsPipelineDesc graphics_pipeline_desc;
	graphics_pipeline_desc.vertex_shader = vertex_shader;
	graphics_pipeline_desc.fragment_shader = fragment_shader;
	graphics_pipeline_desc.bind_group_data = bind_groups;
	graphics_pipeline_desc.bind_group_count = 1;
	graphics_pipeline_desc.push_constant_stage_flags = RHI::GPUShaderStageFlag::All;
	graphics_pipeline_desc.push_constant_size = 256;
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
	RHI::GPUSwapchain* swapchain = RHI::GetRHI()->CreateSwapchain(swapchain_desc);

	while (!GWindowShouldClose)
	{	
		Application::Get().PumpMessageQueue();
	}

	RHI::Shutdown();

	Application::Get().Shutdown();
}