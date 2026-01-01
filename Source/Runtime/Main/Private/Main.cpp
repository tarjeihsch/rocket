#include "Application.h"
#include "MessageController.h"
#include "Logger.h"
#include "Renderer.h"

bool g_window_should_close = false;

class EngineMessageController : public MessageController
{
	void OnWindowClose(Window* Window) override
	{
		g_window_should_close = true;
	}
};

static EngineMessageController G_MessageController;

int main()
{
	Application::get().initialize();
	Application::get().set_message_controller(&G_MessageController);

	Logger::GBufferLog.SetMinLevel(ELogLevel::Trace);

	while (!g_window_should_close)
	{
		Logger::DumpGBufferLogger();

		Application::get().pump_message_queue();

		Renderer::get().BeginFrame();

		//Renderer::get().end_frame();
	}

	//Renderer::get().shutdown();
	Application::get().shutdown();
}