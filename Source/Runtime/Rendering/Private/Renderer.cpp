#include "Renderer.h"

#include "Application.h"
#include "FrameGraph.h"
#include "Viewport.h"
#include "Common/RHI.h"
#include "Common/RHICommandList.h"
#include "Common/RHISurface.h"

static FrameGraph frame_graph;

Renderer::Renderer()
	: m_viewport(nullptr),
	m_viewport_flag(false),
	m_clear_color_flag(false)
{
	SetRHIModule(ERHIInterfaceType::Vulkan);
	GRHI->Init();

	m_viewport = new Viewport();
}

Renderer::~Renderer()
{

}

void Renderer::shutdown()
{
	delete m_viewport;

	GRHI->Shutdown();
}

Renderer& Renderer::get()
{
	static Renderer instance;
	return instance;
}

void Renderer::begin_frame()
{
	m_pending_buffer_uploads.clear();
	m_pending_texture_uploads.clear();

	FrameGraphBuilder frame_graph_builder(frame_graph);

	IRHICommandListContext* ctx = GetRHI()->GetCommandListContext();

	RHISurface* surface = m_viewport->get_surface();
	surface->acquire(*ctx);

	RHITexture* back_buffer = surface->get_back_buffer_texture();

	const uint32_t back_buffer_width = back_buffer->RHIGetTextureDesc().Extent[0];
	const uint32_t back_buffer_height = back_buffer->RHIGetTextureDesc().Extent[1];

	FGTextureState back_buffer_initial_state = { back_buffer->current_layout, back_buffer->current_access, back_buffer->current_pipeline_stage };
	FGTextureState back_buffer_final_state = { ERHITextureLayout::Present, ERHIResourceAccess::None, ERHIPipelineStage::BottomOfPipe };

	FGTextureHandle back_buffer_texture_handle = frame_graph_builder.RegisterExternalTexture("BackBuffer", back_buffer, nullptr, back_buffer_initial_state, back_buffer_final_state);
	FRHITextureViewDesc back_buffer_view_desc = FRHITextureViewDesc::Create2D(back_buffer->RHIGetTextureDesc().Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient);
	FGTextureViewHandle back_buffer_view_handle = frame_graph_builder.CreateTextureView("BackBufferView", back_buffer_view_desc, back_buffer_texture_handle);

	FRHITextureDesc AlbedoDesc = FRHITextureDesc::Create2D(back_buffer_width, back_buffer_height, ERHIResourceLifetime::Transient, ERHITextureFormat::R8G8B8A8_UNORM, ERHITextureUsage::Color | ERHITextureUsage::SRV, ERHITextureLayout::Undefined, 1, 1, 1);
	FRHITextureDesc NormalDesc = FRHITextureDesc::Create2D(back_buffer_width, back_buffer_height, ERHIResourceLifetime::Transient, ERHITextureFormat::R8G8B8A8_UNORM, ERHITextureUsage::Color | ERHITextureUsage::SRV, ERHITextureLayout::Undefined, 1, 1, 1);
	FRHITextureDesc DepthDesc = FRHITextureDesc::Create2D(back_buffer_width, back_buffer_height, ERHIResourceLifetime::Transient, ERHITextureFormat::D32_SFLOAT, ERHITextureUsage::Depth | ERHITextureUsage::SRV, ERHITextureLayout::Undefined, 1, 1, 1);
	FGTextureHandle albedo_texture_handle = frame_graph_builder.CreateTexture("GBuffer.Albedo", AlbedoDesc);
	FGTextureHandle normal_texture_handle = frame_graph_builder.CreateTexture("GBuffer.Normal", NormalDesc);
	FGTextureHandle depth_texture_handle = frame_graph_builder.CreateTexture("GBuffer.Depth", DepthDesc);
	FGTextureViewHandle albedo_rtv_handle = frame_graph_builder.CreateTextureView("GBuffer.Albedo.RTV", FRHITextureViewDesc::Create2D(AlbedoDesc.Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), albedo_texture_handle);
	FGTextureViewHandle normal_rtv_handle = frame_graph_builder.CreateTextureView("GBuffer.Normal.RTV", FRHITextureViewDesc::Create2D(NormalDesc.Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), normal_texture_handle);
	FGTextureViewHandle depth_dsv_handle = frame_graph_builder.CreateTextureView("GBuffer.Depth.DSV", FRHITextureViewDesc::Create2D(DepthDesc.Format, ERHITextureAspect::Depth, ERHIResourceLifetime::Transient), depth_texture_handle);
	FGTextureViewHandle albedo_srv_handle = frame_graph_builder.CreateTextureView("GBuffer.Albedo.SRV", FRHITextureViewDesc::Create2D(AlbedoDesc.Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), albedo_texture_handle);
	FGTextureViewHandle normal_srv_handle = frame_graph_builder.CreateTextureView("GBuffer.Normal.SRV", FRHITextureViewDesc::Create2D(NormalDesc.Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), normal_texture_handle);
	FGTextureViewHandle depth_srv_handle = frame_graph_builder.CreateTextureView("GBuffer.Depth.SRV", FRHITextureViewDesc::Create2D(DepthDesc.Format, ERHITextureAspect::Depth, ERHIResourceLifetime::Transient), depth_texture_handle);

	FRHITextureDesc HDRDesc = FRHITextureDesc::Create2D(back_buffer_width, back_buffer_height, ERHIResourceLifetime::Transient, ERHITextureFormat::R8G8B8A8_UNORM, ERHITextureUsage::Color | ERHITextureUsage::SRV, ERHITextureLayout::Undefined, 1, 1, 1);
	FGTextureHandle RGHDR = frame_graph_builder.CreateTexture("Lighting.HDR", HDRDesc);
	FGTextureViewHandle RGHDRRTV = frame_graph_builder.CreateTextureView("Lighting.HDR.RTV", FRHITextureViewDesc::Create2D(HDRDesc.Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), RGHDR);
	FGTextureViewHandle RGHDRSRV = frame_graph_builder.CreateTextureView("Lighting.HDR.SRV", FRHITextureViewDesc::Create2D(HDRDesc.Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), RGHDR);

	FRHIBufferDesc FrameCBDesc = {};
	FrameCBDesc.Size = 256;
	FrameCBDesc.Lifetime = ERHIResourceLifetime::Transient;
	FrameCBDesc.Memory = ERHIResourceMemory::HostVisible;
	FrameCBDesc.Usage = ERHIBufferUsage::Storage | ERHIBufferUsage::TransferDst;
	FRGBufferHandle RGFrameCB = frame_graph_builder.CreateBuffer("FrameCB", FrameCBDesc);

	FRHIBufferDesc IndirectDesc = {};
	IndirectDesc.Size = 1024;
	IndirectDesc.Lifetime = ERHIResourceLifetime::Transient;
	IndirectDesc.Memory = ERHIResourceMemory::HostVisible;
	IndirectDesc.Usage = ERHIBufferUsage::Storage | ERHIBufferUsage::Indirect;
	FRGBufferHandle RGIndirectArgs = frame_graph_builder.CreateBuffer("IndirectArgs", IndirectDesc);

	frame_graph_builder.AddPass("UploadFrameConstants",
	[&](FrameGraphBuilder& B, FGPass& Pass)
	{
		B.WriteBuffer(Pass, RGFrameCB, ERGBufferUsage::CopyDst);
	},
	[&](IRHICommandListContext& CmdListCtx)
	{
		FRHIBuffer* FrameCB = frame_graph_builder.ResolveBuffer(RGFrameCB);

		struct FFrameConstants { /* viewProj, cameraPos, etc */ uint8_t Data[256]; } Constants = {};

		FrameCB->Map(&Constants, sizeof(Constants), 0);
	});
	
	frame_graph_builder.AddPass("CullCompute",
	[&](FrameGraphBuilder& B, FGPass& Pass)
	{
		B.ReadBuffer(Pass, RGFrameCB, ERGBufferUsage::StorageRead);
		B.WriteBuffer(Pass, RGIndirectArgs, ERGBufferUsage::StorageWrite);
	},
	[&](IRHICommandListContext& CmdListCtx)
	{
		// Bind compute pipeline, bind FrameCB + IndirectArgs (as storage), dispatch...
		// CmdListCtx.RHISetPipeline(CullPipeline);
		// CmdListCtx.RHIBindStorageBuffer(0, Builder.ResolveBuffer(RGFrameCB));
		// CmdListCtx.RHIBindStorageBuffer(1, Builder.ResolveBuffer(RGIndirectArgs));
		// CmdListCtx.RHIDispatch(...);
	});

	frame_graph_builder.AddPass("GBuffer",
	[&](FrameGraphBuilder& B, FGPass& Pass)
	{
		B.WriteTexture(Pass, albedo_texture_handle, ERGTextureUsage::ColorAttachment);
		B.WriteTexture(Pass, normal_texture_handle, ERGTextureUsage::ColorAttachment);
		B.WriteTexture(Pass, depth_texture_handle, ERGTextureUsage::DepthStencilAttachment);

		B.ReadBuffer(Pass, RGFrameCB, ERGBufferUsage::Uniform);
		B.ReadBuffer(Pass, RGIndirectArgs, ERGBufferUsage::Indirect);
	},
	[&](IRHICommandListContext& CmdListCtx)
	{
		FRHITextureView* AlbedoRTV = frame_graph_builder.ResolveTextureView(albedo_rtv_handle);
		FRHITextureView* NormalRTV = frame_graph_builder.ResolveTextureView(normal_rtv_handle);
		FRHITextureView* DepthDSV = frame_graph_builder.ResolveTextureView(depth_dsv_handle);

		std::array<FRHITextureView*, 2> Colors = { AlbedoRTV, NormalRTV };
		CmdListCtx.RHIBeginRendering(Colors, DepthDSV, /*bClear*/true);

		// Bind graphics pipeline, bind FrameCB, then draw using indirect args...
		// CmdListCtx.RHISetPipeline(GBufferPipeline);
		// CmdListCtx.RHIBindUniformBuffer(0, Builder.ResolveBuffer(RGFrameCB));
		// CmdListCtx.RHIDrawIndexedIndirect(Builder.ResolveBuffer(RGIndirectArgs), ...);

		CmdListCtx.RHIEndRendering();
	});
	
	frame_graph_builder.AddPass("Lighting",
	[&](FrameGraphBuilder& B, FGPass& Pass)
	{
		B.ReadTexture(Pass, albedo_texture_handle, ERGTextureUsage::ShaderRead);
		B.ReadTexture(Pass, normal_texture_handle, ERGTextureUsage::ShaderRead);
		B.ReadTexture(Pass, depth_texture_handle, ERGTextureUsage::ShaderRead);

		B.ReadBuffer(Pass, RGFrameCB, ERGBufferUsage::Uniform);

		B.WriteTexture(Pass, RGHDR, ERGTextureUsage::ColorAttachment);
	},
	[&](IRHICommandListContext& CmdListCtx)
	{
		FRHITextureView* HDRRTV = frame_graph_builder.ResolveTextureView(RGHDRRTV);
		std::array<FRHITextureView*, 1> Colors = { HDRRTV };

		CmdListCtx.RHIBeginRendering(Colors, nullptr, /*bClear*/true);

		// Bind lighting pipeline, bind SRVs for Albedo/Normal/Depth, draw fullscreen triangle...
		// CmdListCtx.RHISetPipeline(LightingPipeline);
		// CmdListCtx.RHIBindSampledTexture(0, Builder.ResolveTextureView(RGAlbedoSRV));
		// CmdListCtx.RHIBindSampledTexture(1, Builder.ResolveTextureView(RGNormalSRV));
		// CmdListCtx.RHIBindSampledTexture(2, Builder.ResolveTextureView(RGDepthSRV));
		// CmdListCtx.RHIDraw(3, 1, 0, 0);

		CmdListCtx.RHIEndRendering();
	});

	frame_graph_builder.AddPass("Tonemap",
	[&](FrameGraphBuilder& B, FGPass& Pass)
	{
		B.ReadTexture(Pass, RGHDR, ERGTextureUsage::ShaderRead);
		B.WriteTexture(Pass, back_buffer_texture_handle, ERGTextureUsage::ColorAttachment);
	},
	[&](IRHICommandListContext& CmdListCtx)
	{
		FRHITextureView* BackBufferView = frame_graph_builder.ResolveTextureView(back_buffer_view_handle);

		std::array<FRHITextureView*, 1> Colors = { BackBufferView };
		CmdListCtx.RHIBeginRendering(Colors, nullptr, /*bClear*/false);

		// Bind tonemap pipeline, bind HDR SRV, draw fullscreen triangle...
		// CmdListCtx.RHISetPipeline(TonemapPipeline);
		// CmdListCtx.RHIBindSampledTexture(0, Builder.ResolveTextureView(RGHDRSRV));
		// CmdListCtx.RHIDraw(3, 1, 0, 0);

		CmdListCtx.RHIEndRendering();
	});

	frame_graph_builder.AddPass("BackBufferToPresent",
	[&](FrameGraphBuilder& B, FGPass& Pass)
	{
		B.ReadTexture(Pass, back_buffer_texture_handle, ERGTextureUsage::Present);
	},
	[surface](IRHICommandListContext& ctx)
	{
		surface->present(ctx);
	});

	IRHICommandListContext* cmd_list_ctx = GetRHI()->GetCommandListContext();
	RHICommandList cmd_list(*cmd_list_ctx);

	frame_graph.compile();
	frame_graph.execute(*cmd_list_ctx);

	RHICommandListExecutor::execute(cmd_list_ctx->GetLowestLevelContext(), cmd_list.Flush());

	GRHI->submit();

	frame_graph.Reset();

	GRHI->reset();
}

void Renderer::end_frame()
{

}

void Renderer::clear_color(ClearColor clear_color)
{
}