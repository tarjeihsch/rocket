#include "Renderer.h"

#include "RenderTarget.h"
#include "Viewport.h"
#include "Common/RHI.h"
#include "Common/RHISurface.h"
#include "Common/RHICommandList.h"
#include "Common/RHIPipeline.h"
#include "Common/RHISampler.h"
#include "RenderGraph/RenderGraph.h"
#include "RenderGraph/RenderGraphBuilder.h"
#include <Pipeline.h>
#include <Sampler.h>

Renderer& Renderer::Get()
{
	static Renderer renderer;
	return renderer;
}
static RHISampler* sampler;
void Renderer::Initialize()
{
	SetRHIModule(RHIInterfaceType::Vulkan);
	GetRHI()->Init();

	render_graph = new RenderGraph();

	RHISamplerDesc SamplerDesc = {};
	SamplerDesc.MinFilter = ERHIFilterMode::Linear;
	SamplerDesc.MagFilter = ERHIFilterMode::Linear;
	SamplerDesc.MipFilter = ERHIFilterMode::Linear;
	SamplerDesc.AddressU = ERHIAddressMode::ClampToEdge;
	SamplerDesc.AddressV = ERHIAddressMode::ClampToEdge;
	SamplerDesc.AddressW = ERHIAddressMode::ClampToEdge;
	SamplerDesc.MinLod = 0.0f;
	SamplerDesc.MaxLod = 0.0f;
	sampler = GetRHI()->RHICreateSampler(SamplerDesc);
}

void Renderer::Shutdown()
{
	GetRHI()->Shutdown();
}

void Renderer::BeginFrame(Viewport* viewport)
{
	ensure(state == ST_IsFinished);

	// Reset previously recorded render graph state
	render_graph->Reset();

	// Perform RHI internal GC and wait for rendering to complete
	GetRHI()->Reset();

	// Build new transient frame data
	context.target_surface = viewport->GetSurface();
	context.target_surface->AcquireBackBuffer();

	render_graph->ImportTexture("BackBuffer",
		context.target_surface->GetBackBufferTexture(),
		{ context.target_surface->GetImageAvailableSemaphore().get() },
		{ context.target_surface->GetRenderFinishedSemaphore().get() }
	);

	state = ST_HasBegun;
}

void Renderer::EndFrame()
{
	ensure(state == ST_HasBegun);

	{
		GraphicsPipeline ACESTonemap(ROCKET_BINARIES "/Shaders/ACESToneMap.vert.spv", ROCKET_BINARIES "/Shaders/ACESToneMap.frag.spv");

		RHISamplerDesc SamplerDesc = {};
		SamplerDesc.MinFilter = ERHIFilterMode::Linear;
		SamplerDesc.MagFilter = ERHIFilterMode::Linear;
		SamplerDesc.MipFilter = ERHIFilterMode::Linear;
		SamplerDesc.AddressU = ERHIAddressMode::ClampToEdge;
		SamplerDesc.AddressV = ERHIAddressMode::ClampToEdge;
		SamplerDesc.AddressW = ERHIAddressMode::ClampToEdge;
		SamplerDesc.MinLod = 0.0f;
		SamplerDesc.MaxLod = 0.0f;

		RHITextureDesc ColorDesc = RHITextureDesc::CreateColorRT(1280, 720, RHITextureFormat::R8G8B8A8_UNORM);
		RHITextureDesc DepthDesc = RHITextureDesc::CreateDepthRT(1280, 720, RHITextureFormat::D32_SFLOAT_S8_UINT);

		TextureHandle hSceneColor = render_graph->CreateTexture("SceneColor", ColorDesc);
		TextureHandle hSceneDepth = render_graph->CreateTexture("SceneDepth", DepthDesc);
		TextureHandle hBackBuffer = render_graph->GetTextureHandle("BackBuffer");

		render_graph->AddGraphicsPass("GBuffer",
			[&, hSceneColor, hSceneDepth](RenderGraphBuilder& Builder)
			{
				Builder.WriteColorAttachment(hSceneColor);
				Builder.WriteDepthAttachment(hSceneDepth);
			},
			[=](RHICommandList& command_list)
			{
				RHITexture* pColor = render_graph->ResolveTexture(hSceneColor);
				RHITexture* pDepth = render_graph->ResolveTexture(hSceneDepth);

				RHITextureView* pColorView = pColor->GetDefaultView();
				RHITextureView* pDepthView = pDepth->GetDefaultView();

				RHIRenderPassInfo renderPassInfo = {};
				renderPassInfo.ColorRenderTargets[0].TextureView = pColorView;
				renderPassInfo.ColorRenderTargets[0].LoadOp = LoadAction::Clear;
				renderPassInfo.ColorRenderTargets[0].StoreOp = StoreAction::Store;
				renderPassInfo.ColorRenderTargets[0].ClearValue = LinearColor::Green;
				renderPassInfo.NumColorRenderTargets = 1;
				
				renderPassInfo.DepthStencilRenderTarget.TextureView = pDepthView;
				renderPassInfo.DepthStencilRenderTarget.DepthLoadOp = LoadAction::Clear;
				renderPassInfo.DepthStencilRenderTarget.DepthStoreOp = StoreAction::Store;
				renderPassInfo.DepthStencilRenderTarget.ClearDepth = 1.0f;
				renderPassInfo.DepthStencilRenderTarget.ClearStencil = 0.0f;

				command_list.Enqueue<RHICommandBeginRendering>(renderPassInfo);

				command_list.Enqueue<RHICommandSetScissor>(Vector2i(0, 0), Vector2i(1280, 720));
				command_list.Enqueue<RHICommandSetViewport>(Vector2i(0, 0), Vector2i(1280, 720));

				command_list.Enqueue<RHICommandEndRendering>();
			}
		);

		render_graph->AddGraphicsPass("Composition",
			[&](RenderGraphBuilder& Builder)
			{
				Builder.ReadTexture(hSceneColor);
				Builder.WriteColorAttachment(hBackBuffer);
			},
			[=](RHICommandList& command_list)
			{
				RHITexture* pColor = render_graph->ResolveTexture(hSceneColor);
				RHITexture* pBackBuffer = render_graph->ResolveTexture(hBackBuffer);

				RHITextureView* pColorView = pColor->GetDefaultView();
				RHITextureView* pBackBufferView = pBackBuffer->GetDefaultView();

				RHIRenderPassInfo RenderPassInfo;
				RenderPassInfo.ColorRenderTargets[0].TextureView = pBackBufferView;
				RenderPassInfo.ColorRenderTargets[0].LoadOp = LoadAction::DontCare;
				RenderPassInfo.ColorRenderTargets[0].StoreOp = StoreAction::Store;
				RenderPassInfo.ColorRenderTargets[0].ClearValue = LinearColor::Green;
				RenderPassInfo.NumColorRenderTargets = 1;

				RHIBufferMemoryPacker MemoryPacker(RHIBufferMemoryPackerAlignmentLayout::STD430);
				MemoryPacker.Write(pColorView->GetBindingIndex());
				MemoryPacker.Write(sampler->GetBindingIndex());

				command_list.Enqueue<RHICommandBeginRendering>(RenderPassInfo);

				command_list.Enqueue<RHICommandBindPipeline>(ACESTonemap.GetRHIPipeline());
				command_list.Enqueue<RHICommandBindGlobalHeap>();
				command_list.Enqueue<RHICommandSetShaderParameters>(MemoryPacker.Copy());
				command_list.Enqueue<RHICommandSetViewport>(Vector2i(0, 0), Vector2i(1280, 720));
				command_list.Enqueue<RHICommandSetScissor>(Vector2i(0, 0), Vector2i(1280, 720));
				command_list.Enqueue<RHICommandDraw>(3, 1, 0, 0);

				command_list.Enqueue<RHICommandEndRendering>();
			}
		);

		render_graph->AddGraphicsPass("Present",
			[&](RenderGraphBuilder& Builder)
			{
				Builder.Present(hBackBuffer);
			},
			[](RHICommandList& command_list)
			{
			}
		);
	}

	render_graph->Compile();
	render_graph->Execute();

	GetRHI()->Submit(render_graph->GetCommandList());

	context.target_surface->PresentBackBuffer();

	state = ST_IsFinished;
}