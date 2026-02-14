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

	while (!GWindowShouldClose)
	{	
		Application::Get().PumpMessageQueue();
	}

	RHI::Shutdown();

	Application::Get().Shutdown();
}