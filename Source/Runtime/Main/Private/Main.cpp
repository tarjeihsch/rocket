#include "Application.h"
#include "MessageController.h"
#include "Common/RHI.h"

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

	{
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

		RHI::GPUBindGroup* bind_group = RHI::GetRHI()->CreateBindGroup(bind_group_desc);
		bind_group->SetTexture(texture, 0, 0);
	}


	while (!GWindowShouldClose)
	{	
		Application::Get().PumpMessageQueue();
	}

	RHI::Shutdown();

	Application::Get().Shutdown();
}