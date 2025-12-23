#include "Application.h"
#include "Window.h"
#include "MessageController.h"

#include <memory>

class EngineMessageController : public MessageController
{
};

static EngineMessageController g_MessageController;

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
	Application::Get().SetMessageController(&g_MessageController);

	// Create and forward a surface
	std::shared_ptr<Window> window = Application::Get().CreateWindow(windowCreateInfo);

	while (true)
	{
		Application::Get().PumpMessageQueue();
	}
}