#include "EditorRenderer.h"

#include "Viewport.h"
#include "FrameGraph.h"
#include "RenderTarget.h"
#include "Common/RHIDrawCommand.h"
#include <memory>
#include <Common/RHI.h>
#include "../imgui/imgui.h"
#include <Input.h>
#include <KeyCode.h>
#include <Pipeline.h>
#include "Common/RHIBuffer.h"
#include "Common/RHISampler.h"
#include "Common/RHITexture.h"

static inline float ImFloor(float f) { return (float)((f >= 0 || (float)(int)f == f) ? (int)f : (int)f - 1); } // Decent replacement for floorf()

REditorRenderer::REditorRenderer()
{
	Init();
}

REditorRenderer::~REditorRenderer()
{
	Shutdown();
}

void REditorRenderer::Init()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)1280, (float)720);
	
	unsigned char* pixels;
	int width, height;
	ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	
	ImGuiTextureAtlas = GetRHI()->RHICreateTexture(FRHITextureDesc::Create2D(width, height, ERHIResourceLifetime::Persistent, ERHITextureFormat::R8G8B8A8_UNORM, ERHITextureUsage::SRV | ERHITextureUsage::CopyDst, ERHITextureLayout::Undefined, 0, 1, 0));
	
	GetRHI()->RHIUploadTextureBlocking(ImGuiTextureAtlas, pixels, width, height);
}

void REditorRenderer::Shutdown()
{
	delete ImGuiTextureAtlas;
}

void REditorRenderer::Render(float DeltaTime)
{
    // Draw ImGui elements

	FFrameGraphBuilder Builder;

	FRenderTarget RenderTarget = FRenderTarget(GetRHI()->RHIGetViewportExtent()[0], GetRHI()->RHIGetViewportExtent()[1]);

	static bool a = true;

	std::unique_ptr<FRHIBuffer> VertexBuffer;
	std::unique_ptr<FRHIBuffer> IndexBuffer;
	std::unique_ptr<FRHITextureView> TextureView;
	std::unique_ptr<RRHISampler> Sampler;

	ImGuiIO& io = ImGui::GetIO();
	const auto extent = GetRHI()->RHIGetViewportExtent();
	io.DisplaySize = ImVec2((float)extent[0], (float)extent[1]);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.DeltaTime = DeltaTime;
	io.MousePos = ImVec2((float)PInput::GetMouseX(), (float)PInput::GetMouseY());
	io.MouseDown[0] = PInput::GetMouseButtonPressed(RK_MOUSE_BUTTON_LEFT);
	io.MouseDown[1] = PInput::GetMouseButtonPressed(RK_MOUSE_BUTTON_RIGHT);

	ImGui::NewFrame();
	ImGui::ShowDemoWindow(&a);
	ImGui::Render();
	ImDrawData* DrawData = ImGui::GetDrawData();

	FRHIPipelineDesc ImGuiPipelineDesc = {};
	ImGuiPipelineDesc.VertexShaderPath = ROCKET_SRC "/Shader/ImGuiVS.hlsl";
	ImGuiPipelineDesc.PixelShaderPath = ROCKET_SRC "/Shader/ImGuiPS.hlsl";
	ImGuiPipelineDesc.bDepthTestEnable = false;
	ImGuiPipelineDesc.bDepthWriteEnable = false;
	ImGuiPipelineDesc.CullMode = ERHICullMode::None;
	ImGuiPipelineDesc.ColorAttachmentFormats.push_back(ERHIFormat::BGRA8_SRGB);
	ImGuiPipelineDesc.VertexBindings.push_back(FRHIVertexBinding(0, 20, 0));
	ImGuiPipelineDesc.VertexAttributes.emplace_back(0, 0, ERHIFormat::Float2, 0);  // pos
	ImGuiPipelineDesc.VertexAttributes.emplace_back(1, 0, ERHIFormat::Float2, 8);  // uv (Float2!)
	ImGuiPipelineDesc.VertexAttributes.emplace_back(2, 0, ERHIFormat::UNorm8x4, 16); // color
	ImGuiPipelineDesc.BlendAttachments.push_back(FRHIBlendAttachment(1, ERHIBlendFactor::SrcAlpha, ERHIBlendFactor::OneMinusSrcAlpha, ERHIBlendOp::Add, ERHIBlendFactor::One, ERHIBlendFactor::OneMinusSrcAlpha, ERHIBlendOp::Add, (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3)));

	FPipeline ImGuiPipeline = FPipeline(ImGuiPipelineDesc);

	FFrameGraphPass& ImGuiPass = Builder.AddPass("ImGui", EFGPassFlags::Graphics | EFGPassFlags::Dynamic | EFGPassFlags::ClearColor, [&](FRHICommandList& RHICmdList)
	{
		GetRHI()->RHICmdBindPipeline(RHICmdList, ImGuiPipeline.GetHandle());

		size_t totalVtx = 0, totalIdx = 0;
		for (int n = 0; n < DrawData->CmdListsCount; ++n)
		{
			totalVtx += DrawData->CmdLists[n]->VtxBuffer.Size;
			totalIdx += DrawData->CmdLists[n]->IdxBuffer.Size;
		}

		const size_t vbBytes = totalVtx * sizeof(ImDrawVert);
		const size_t ibBytes = totalIdx * sizeof(ImDrawIdx);

		if (!(vbBytes > 0 && ibBytes > 0)) return;

		FRHIBufferDesc VertexBufferDesc = { vbBytes, ERHIResourceLifetime::Transient, ERHIBufferUsage::Vertex, ERHIResourceMemory::HostVisible };
		VertexBuffer.reset(GetRHI()->RHICreateBuffer(VertexBufferDesc));

		FRHIBufferDesc IndexBufferDesc = { ibBytes, ERHIResourceLifetime::Transient, ERHIBufferUsage::Index, ERHIResourceMemory::HostVisible };
		IndexBuffer.reset(GetRHI()->RHICreateBuffer(IndexBufferDesc));
		
		RRHISamplerDesc desc{};
		desc.MinFilter = ERHIFilterMode::Linear;
		desc.MagFilter = ERHIFilterMode::Linear;
		desc.MipFilter = ERHIFilterMode::Linear;
		desc.AddressU = ERHIAddressMode::ClampToEdge;
		desc.AddressV = ERHIAddressMode::ClampToEdge;
		desc.AddressW = ERHIAddressMode::ClampToEdge;
		desc.MinLod = 0.0f;
		desc.MaxLod = 0.0f;

		size_t vbOff = 0, ibOff = 0;
		std::vector<size_t> listVtxBase(DrawData->CmdListsCount), listIdxBase(DrawData->CmdListsCount);

		for (int n = 0; n < DrawData->CmdListsCount; ++n)
		{
			const ImDrawList* cl = DrawData->CmdLists[n];
			const size_t vsz = cl->VtxBuffer.Size * sizeof(ImDrawVert);
			const size_t isz = cl->IdxBuffer.Size * sizeof(ImDrawIdx);
			VertexBuffer->Map(cl->VtxBuffer.Data, vsz, vbOff);
			IndexBuffer->Map(cl->IdxBuffer.Data, isz, ibOff);
			listVtxBase[n] = vbOff / sizeof(ImDrawVert);
			listIdxBase[n] = ibOff / sizeof(ImDrawIdx);
			vbOff += vsz; ibOff += isz;
		}

		struct Push { glm::vec2 scale; glm::vec2 translate; } pc;
		pc.scale.x = 2.0f / DrawData->DisplaySize.x;
		pc.scale.y = 2.0f / DrawData->DisplaySize.y;
		pc.translate.x = -1.0f - DrawData->DisplayPos.x * pc.scale.x;
		pc.translate.y = -1.0f - DrawData->DisplayPos.y * pc.scale.y;

		for (int n = 0; n < DrawData->CmdListsCount; ++n)
		{
			const ImDrawList* cl = DrawData->CmdLists[n];
			size_t idxOffset = listIdxBase[n];
			size_t vtxBase = listVtxBase[n];

			for (const ImDrawCmd& cmd : cl->CmdBuffer)
			{
				if (cmd.UserCallback) { cmd.UserCallback(cl, &cmd); continue; }

				FRHITextureViewDesc TextureViewDesc;
				TextureViewDesc.Format = ERHITextureFormat::R8G8B8A8_UNORM;
				TextureViewDesc.Aspect = ERHITextureAspect::Color;
				TextureViewDesc.Lifetime = ERHIResourceLifetime::Transient;
				TextureView.reset(GetRHI()->RHICreateTextureView(TextureViewDesc, ImGuiTextureAtlas));
				Sampler.reset(GetRHI()->RHICreateSampler(desc));

				ImVec2 clipMin = (ImVec2(ImFloor((cmd.ClipRect.x - DrawData->DisplayPos.x) * DrawData->FramebufferScale.x), ImFloor((cmd.ClipRect.y - DrawData->DisplayPos.y) * DrawData->FramebufferScale.y)));
				ImVec2 clipMax = (ImVec2(ImFloor((cmd.ClipRect.z - DrawData->DisplayPos.x) * DrawData->FramebufferScale.x), ImFloor((cmd.ClipRect.w - DrawData->DisplayPos.y) * DrawData->FramebufferScale.y)));
				if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y) { idxOffset += cmd.ElemCount; continue; }

				GetRHI()->RHICmdSetScissor(RHICmdList, (int32_t)clipMin.x, (int32_t)clipMin.y, (uint32_t)(clipMax.x - clipMin.x), (uint32_t)(clipMax.y - clipMin.y));

				FRHIDrawCommand dc = {};
				dc.IndexCount = cmd.ElemCount;
				dc.IndexBuffer = IndexBuffer.get();
				dc.VertexBuffer = VertexBuffer.get();
				dc.IndexBufferOffset = cmd.IdxOffset + listIdxBase[n];
				dc.VertexBufferOffset = cmd.VtxOffset + listVtxBase[n];
				dc.IndexType = FRHIDrawCommand::EIndexType::U16;

				FRHIDrawParams dp = {};
				dp.DrawParam_U32_1 = TextureView->GetBindingIndex();
				dp.DrawParam_U32_2 = Sampler->GetBindingIndex();
				dp.DrawParam_F32_1 = pc.scale[0];
				dp.DrawParam_F32_2 = pc.scale[1];
				dp.DrawParam_F32_3 = pc.translate[0];
				dp.DrawParam_F32_4 = pc.translate[1];
				

				GetRHI()->RHICmdDrawIndexed(RHICmdList, dc, dp);

				idxOffset += cmd.ElemCount;
			}
		}
	});
	ImGuiPass.AddColorAttachment(RenderTarget.GetHandle());



	// These lines should be covered by the Execute function itself. Should not have to specify the composition and present passes manually.
	FFrameGraphPass& CompositionPass = Builder.AddPass("Composition", EFGPassFlags::Compute, [&](FRHICommandList& RHICmdList)
	{
		GetRHI()->RHICmdBlitImage(RHICmdList, RenderTarget.GetHandle(), GetRHI()->RHIGetBackBufferRenderTarget());
	});

	CompositionPass.AddTransferSrc(RenderTarget.GetHandle());
	CompositionPass.AddTransferDst(GetRHI()->RHIGetBackBufferRenderTarget());

	FFrameGraphPass& PresentPass = Builder.AddPass("Present", EFGPassFlags::Graphics, nullptr);
	PresentPass.AddPresentSrc(GetRHI()->RHIGetBackBufferRenderTarget());




	// This should be a unified call done by FrameGraph itself, not the builder.
	// Should take all the builder's and combine them. (eg. Editor, Game)
	Builder.Execute();
}