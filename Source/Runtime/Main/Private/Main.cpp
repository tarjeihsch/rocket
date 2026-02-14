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

	while (!GWindowShouldClose)
	{
		Application::Get().PumpMessageQueue();
	}

	RHI::Shutdown();

	Application::Get().Shutdown();
}