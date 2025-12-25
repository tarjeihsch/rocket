#include "Application.h"
#include "Window.h"
#include "MessageController.h"
#include "Display.h"
#include "Logger.h"

bool shouldClose = false;

class EngineMessageController : public MessageController
{
	void OnWindowClose(RWindow* Window) override
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

	RDisplay* Display = new RDisplay();

	Logger::GBufferLog.SetMinLevel(ELogLevel::Trace);

	while (!shouldClose)
	{
		Logger::DumpGBufferLogger();

		Application::Get().PumpMessageQueue();
	}

	Application::Get().Shutdown();
}