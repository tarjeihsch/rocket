#include "Application.h"
#include "Window.h"
#include "MessageController.h"

bool shouldClose = false;

class EngineMessageController : public MessageController
{
	void OnWindowClose(Platform::Window* Window) override
	{
		shouldClose = true;
	}
};

static EngineMessageController G_MessageController;

int main()
{
	WindowCreateInfo windowCreateInfo = {};
	windowCreateInfo.title = "Rocket Engine";
	windowCreateInfo.width = 1280;
	windowCreateInfo.height = 720;
	windowCreateInfo.visible = true;
	windowCreateInfo.hdpi = true;
	windowCreateInfo.resizable = true;

	// Initialize platform layer
	Application::Get().Initialize();
	Application::Get().SetMessageController(&G_MessageController);

	// Create and forward a surface
	Platform::Window* window = Application::Get().CreateWindow(windowCreateInfo);

	while (!shouldClose)
	{
		Application::Get().PumpMessageQueue();
	}

	Application::Get().Shutdown();
}