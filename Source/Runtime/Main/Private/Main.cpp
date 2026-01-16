#include "Application.h"
#include "MessageController.h"
#include "Logger.h"
#include "Renderer.h"
#include "RenderTarget.h"
#include "Attachment.h"
#include "Buffer.h"
#include "Pipeline.h"
#include "Viewport.h"

bool g_window_should_close = false;

class EngineMessageController : public MessageController
{
	void OnWindowClose(OSWindow* Window) override
	{
		g_window_should_close = true;
	}
};

static EngineMessageController g_message_controller;
static Viewport* game_viewport;

int main()
{
	Application::Get().Initialize();
	Application::Get().SetMessageController(&g_message_controller);

	Renderer::Get().Initialize();

	game_viewport = new Viewport();

	Logger::GBufferLog.SetMinLevel(ELogLevel::Trace);

	while (!g_window_should_close)
	{
		Logger::DumpGBuffer();

		Application::Get().PumpMessageQueue();

		Renderer::Get().BeginFrame(game_viewport);

		Renderer::Get().EndFrame();
	}

	Application::Get().Shutdown();
}