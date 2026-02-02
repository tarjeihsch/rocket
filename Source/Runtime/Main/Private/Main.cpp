#include "Application.h"
#include "MessageController.h"

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

	while (!GWindowShouldClose)
	{
		Application::Get().PumpMessageQueue();
	}

	Application::Get().Shutdown();
}