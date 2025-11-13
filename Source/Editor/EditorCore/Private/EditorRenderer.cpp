#include "EditorRenderer.h"

#include "Renderer.h"
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
#include <Logger.h>
#include "Texture2D.h"
#include <Material.h>
#include <MathTypes.h>
#include <Editor.h>
#include <Actor.h>
#include "World.h"
#include "EditorBase.h"
#include <PrimitiveManager.h>
#include "StaticMesh.h"
#include "Attachment.h"
#include "StaticMeshComponent.h"

static RActor* SelectedActor = nullptr;
static RComponent* SelectedComponent = nullptr;
static bool S_DockspaceOpen = true;
static bool S_Fullscreen = true;
static bool S_Padding = false;
static bool SShowStatsWindow = false;
static ImGuiDockNodeFlags S_DockspaceFlags = ImGuiDockNodeFlags_None;
static float FrameTimes[120] = {};
static int FrameIndex = 0;

REditorViewportClient::REditorViewportClient()
{
	Init();
}

REditorViewportClient::~REditorViewportClient()
{
	Shutdown();
}

void REditorViewportClient::Init()
{
	ImGui::CreateContext();
	ImGuiIO& IO = ImGui::GetIO();
	IO.DisplaySize = ImVec2((float)1280, (float)720);
	IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	unsigned char* Data;
	int Width, Height;
	ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&Data, &Width, &Height);
	
	ImGui_FontAtlas = std::make_unique<RTexture2D>(ETextureType::Linear, Width, Height);
	ImGui_FontAtlas->LoadRawTextureData(Data);
	IO.Fonts->SetTexID((ImTextureID)ImGui_FontAtlas->GetHandle());
}

void REditorViewportClient::Shutdown()
{
	ImGui_FontAtlas = nullptr;
}

void REditorViewportClient::Draw(RFrameContext& Ctx)
{
	FRenderTarget EditorViewportRenderTarget = FRenderTarget(Ctx.ViewportWidth, Ctx.ViewportHeight);
	FRenderTarget SceneViewportRenderTarget = FRenderTarget(Ctx.ViewportWidth, Ctx.ViewportHeight);
	
	/* ---- Begin ImGUI ---- */

	std::unique_ptr<FRHIBuffer> VertexBuffer;
	std::unique_ptr<FRHIBuffer> IndexBuffer;
	std::unique_ptr<FRHITextureView> TextureView;
	std::unique_ptr<RRHISampler> Sampler;

	ImGuiIO& IO = ImGui::GetIO();
	IO.DisplaySize = ImVec2(Ctx.ViewportWidth, Ctx.ViewportHeight);
	IO.DeltaTime = Ctx.Timestep;
	IO.MousePos = ImVec2((float)PInput::GetMouseX(), (float)PInput::GetMouseY());
	IO.MouseDown[0] = PInput::GetMouseButtonPressed(RK_MOUSE_BUTTON_LEFT);
	IO.MouseDown[1] = PInput::GetMouseButtonPressed(RK_MOUSE_BUTTON_RIGHT);

	ImGui::NewFrame();

	ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (S_Fullscreen)
	{
		const ImGuiViewport* MainViewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(MainViewport->WorkPos);
		ImGui::SetNextWindowSize(MainViewport->WorkSize);
		ImGui::SetNextWindowViewport(MainViewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	if (!S_Padding)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	}

	ImGui::Begin("MainDockspace", &S_DockspaceOpen, WindowFlags);
	
	if (!S_Padding)
	{
		ImGui::PopStyleVar();
	}

	if (S_Fullscreen)
	{
		ImGui::PopStyleVar(2);
	}

	if (IO.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID DockspaceID = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(DockspaceID, ImVec2(0.0f, 0.0f), S_DockspaceFlags);
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit")) {}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Object"))
		{
			if (ImGui::MenuItem("New Actor")) 
			{
			    RStaticMesh* StaticMesh = new RStaticMesh("C:/Users/Tarjei/Downloads/Untitled.glb");
			    RActor* Actor = GEditor->GetWorld()->SpawnActor(RActor::StaticClass());
			    RStaticMeshComponent* StaticMeshComponent = Actor->AddComponent<RStaticMeshComponent>();
			    StaticMeshComponent->SetStaticMesh(StaticMesh);
			}

			ImGui::EndMenu();
		}
	    if (ImGui::BeginMenu("Stats"))
	    {
	        SShowStatsWindow = true;
	        ImGui::EndMenu();
	    }

		ImGui::EndMenuBar();
	}

    if (SShowStatsWindow)
    {
        ImGui::Begin("Stats", &SShowStatsWindow);

        float ms = ImGui::GetIO().DeltaTime * 1000.0f;
        FrameTimes[FrameIndex] = ms;
        FrameIndex = (FrameIndex + 1) % IM_ARRAYSIZE(FrameTimes);

        ImGui::Text("Frame Time: %.3f ms", ms);
        ImGui::Text("FPS: %.1f", 1000.0f / ms);

        ImGui::PlotLines("Frame Time (ms)", FrameTimes, IM_ARRAYSIZE(FrameTimes), FrameIndex, nullptr, 0.0f, 50.0f, ImVec2(0, 80));

        ImGui::End();
    }


	ImGui::Begin("Scene Hierarchy");
	std::span<RActor*> Actors = GEditor->GetWorld()->GetActors();
	for (size_t Index = 0; Index < Actors.size(); ++Index)
	{
		RActor* Actor = Actors[Index];
		std::string ActorLabel = "Actor##" + std::to_string(Index);

		if (ImGui::Selectable(ActorLabel.c_str(), (SelectedActor == Actor)))
		{
			SelectedActor = Actor;
			SelectedComponent = nullptr;
		}
	}

	ImGui::End();

	ImGui::Begin("Components");
	if (SelectedActor)
	{
		ImGui::Text("%s", SelectedActor->GetClass()->GetClassName());
		ImGui::Separator();

		std::span<RComponent*> Components = SelectedActor->GetComponents();
		for (size_t Index = 0; Index < Components.size(); ++Index)
		{
			RComponent* Component = Components[Index];
			std::string ComponentLabel = "Component##" + std::to_string(Index);

			if (ImGui::Selectable(ComponentLabel.c_str(), (SelectedComponent == Component)))
			{
				SelectedComponent = Component;
			}
		}
	}
	ImGui::End();

	ImGui::Begin("Properties");
	if (SelectedComponent)
	{
	    ImGui::Text("%s", SelectedComponent->GetClass()->GetClassName());
	    ImGui::Separator();

	    for (const RClass* Class = SelectedComponent->GetClass(); Class != nullptr; Class = Class->GetSuperClass())
	    {
	        std::span<RField> Fields = Class->GetFields();

	        for (const RField& Field : Fields)
	        {
	            switch (Field.FieldType)
	            {

	            case EFieldType::Bool:
	            {
	                bool* Value = static_cast<bool*>(Field.GetValue(SelectedComponent));
	                if (ImGui::Checkbox(Field.Name, Value))
	                {
	                    SelectedComponent->OnPropertyChanged();
	                }
	                break;
	            }

	            case EFieldType::Float:
	            {
	                float* Value = static_cast<float*>(Field.GetValue(SelectedComponent));
	                if (ImGui::DragFloat(Field.Name, Value))
	                {
	                    SelectedComponent->OnPropertyChanged();
	                }
	                break;
	            }

	            case EFieldType::Int:
	            {
	                int32_t* Value = static_cast<int32_t*>(Field.GetValue(SelectedComponent));
	                if (ImGui::DragInt(Field.Name, Value))
	                {
	                    SelectedComponent->OnPropertyChanged();
	                }
	                break;
	            }

	            case EFieldType::Vector:
	            {
	                float* Value = static_cast<float*>(Field.GetValue(SelectedComponent));
	                if (ImGui::DragFloat3(Field.Name, Value))
	                {
                        SelectedComponent->OnPropertyChanged();
	                }
	                break;
	            }

	            default:
	                break;
	            }
	        }
	    }
	}
	ImGui::End();

	ImGui::Begin("Scene Viewport");
	ImGui::Image((ImTextureID)SceneViewportRenderTarget.GetHandle(), ImGui::GetContentRegionAvail());
	ImGui::End();

	ImGui::End();
	ImGui::Render();

	ImDrawData* DrawData = ImGui::GetDrawData();

	FRHIPipelineDesc PipelineSpec = {};
	PipelineSpec.PixelShaderPath = ROCKET_SRC "/Shader/PixelShader.hlsl";
	PipelineSpec.VertexShaderPath = ROCKET_SRC "/Shader/VertexShader.hlsl";
	PipelineSpec.bDepthTestEnable = true;
	PipelineSpec.bDepthWriteEnable = true;
	PipelineSpec.DepthFormat = ERHIFormat::D32_Float_S8X24;
	PipelineSpec.CullMode = ERHICullMode::Back;
	PipelineSpec.ColorAttachmentFormats.emplace_back(ERHIFormat::BGRA8_SRGB);
	PipelineSpec.VertexBindings.emplace_back(FRHIVertexBinding(0, 48, 0));
	PipelineSpec.VertexAttributes.emplace_back(0, 0, ERHIFormat::Float3, static_cast<uint32_t>(offsetof(FRHIVertex, m_Position)));
	PipelineSpec.VertexAttributes.emplace_back(1, 0, ERHIFormat::Float3, static_cast<uint32_t>(offsetof(FRHIVertex, m_Normal)));
	PipelineSpec.VertexAttributes.emplace_back(2, 0, ERHIFormat::Float2, static_cast<uint32_t>(offsetof(FRHIVertex, m_TexCoord)));
	PipelineSpec.BlendAttachments.emplace_back(FRHIBlendAttachment(false, ERHIBlendFactor::SrcAlpha, ERHIBlendFactor::OneMinusSrcAlpha, ERHIBlendOp::Add, ERHIBlendFactor::One, ERHIBlendFactor::Zero, ERHIBlendOp::Add, (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3)));

	FRHIPipeline* Pipeline = GetRHI()->RHICreatePipelineStateObject(PipelineSpec);
	FAttachment DepthAttachment = FAttachment(GetRHI()->RHIGetViewportExtent()[0], GetRHI()->RHIGetViewportExtent()[1], ERHITextureFormat::D32_SFLOAT_S8_UINT, ERHITextureUsage::Depth | ERHITextureUsage::Stencil);

	FFrameGraphPass& SceneColorPass = Ctx.Builder.AddPass("Opaque", EFGPassFlags::Graphics | EFGPassFlags::Dynamic | EFGPassFlags::ClearColor,
		[&](FRHICommandList& RHICmdList)
		{
			GetRHI()->RHICmdBindPipeline(RHICmdList, Pipeline);
			for (const LSceneView& View : GEditor->GetWorld()->GetSceneProxyManager()->GetViews())
			{
				FRHIBufferMemoryPacker CameraData(ERHIBufferMemoryPackerAlignmentLayout::STD430);
				CameraData.Write(View.ViewMatrix);
				CameraData.Write(View.ProjectionMatrix);
				std::shared_ptr<FRHIBuffer> ViewBuffer = GetRHI()->RHIBindBuffer(CameraData.GetData(), CameraData.GetSize());

				for (const LSceneProxy& Proxy : GEditor->GetWorld()->GetSceneProxyManager()->GetProxies())
				{
					RStaticMesh* StaticMesh = Proxy.Geometry.StaticMesh;
					for (const auto& Section : StaticMesh->GetSections())
					{
					    FRHIBufferMemoryPacker ObjectData(ERHIBufferMemoryPackerAlignmentLayout::STD430);
					    ObjectData.Write(Proxy.Transform.ModelMatrix * Section.LocalMatrix);
					    ObjectData.Write(Proxy.Transform.InverseModelMatrix);
					    std::shared_ptr<FRHIBuffer> ObjectInterface_GPUBuffer = GetRHI()->RHIBindBuffer(ObjectData.GetData(), ObjectData.GetSize());
					
					    FRHIDrawCommand DrawCommand = {};
					    DrawCommand.VertexBuffer = StaticMesh->GetVertexBuffer();
					    DrawCommand.IndexBuffer = StaticMesh->GetIndexBuffer();
					    DrawCommand.IndexCount = Section.M_IndexCount;
					    DrawCommand.VertexBufferOffset = Section.M_VertexOffset;
					    DrawCommand.IndexBufferOffset = Section.M_IndexOffset;
					    DrawCommand.IndexType = FRHIDrawCommand::EIndexType::U32;
					
					    FRHIDrawParams DrawParams = {};
					    DrawParams.DrawParam_U32_1 = ViewBuffer->GetBindingIndex(); // Camera ID
						DrawParams.DrawParam_U32_2 = ObjectInterface_GPUBuffer->GetBindingIndex(); // Instance ID
						DrawParams.DrawParam_U32_3 = 0;
						DrawParams.DrawParam_U32_4 = 0;

						DrawParams.DrawParam_F32_1 = 0.0f;
						DrawParams.DrawParam_F32_2 = 0.0f;
						DrawParams.DrawParam_F32_3 = 0.0f;
						DrawParams.DrawParam_F32_4 = 0.0f;

					    GetRHI()->RHICmdDrawIndexed(RHICmdList, DrawCommand, DrawParams);
					}
				}
			}
		});
	SceneColorPass.AddColorAttachment(SceneViewportRenderTarget.GetHandle());
	SceneColorPass.AddDepthAttachment(DepthAttachment.GetHandle());

	FFrameGraphPass& UIPass = Ctx.Builder.AddPass("UI Pass", EFGPassFlags::Graphics | EFGPassFlags::Dynamic | EFGPassFlags::ClearColor, [&](FRHICommandList& RHICmdList)
	{
		std::unique_ptr<RMaterial> UIMaterial = std::make_unique<RMaterial>(EMaterialPass::UI, EMaterialBlendMode::Transparent, ROCKET_SRC "/Shader/ImGuiVS.hlsl", ROCKET_SRC "/Shader/ImGuiPS.hlsl");

		UIMaterial->Bind(RHICmdList);

		size_t TotalVertices = 0;
		size_t TotalIndices = 0;

		for (int Index = 0; Index < DrawData->CmdListsCount; ++Index)
		{
			TotalVertices += DrawData->CmdLists[Index]->VtxBuffer.Size;
			TotalIndices += DrawData->CmdLists[Index]->IdxBuffer.Size;
		}

		FRHIBufferDesc VertexBufferDesc = {};
		VertexBufferDesc.Size = TotalVertices * sizeof(ImDrawVert);
		VertexBufferDesc.Lifetime = ERHIResourceLifetime::Transient;
		VertexBufferDesc.Usage = ERHIBufferUsage::Vertex;
		VertexBufferDesc.Memory = ERHIResourceMemory::HostVisible;
		VertexBuffer.reset(GetRHI()->RHICreateBuffer(VertexBufferDesc));

		FRHIBufferDesc IndexBufferDesc = {};
		IndexBufferDesc.Size = TotalIndices * sizeof(ImDrawIdx);
		IndexBufferDesc.Lifetime = ERHIResourceLifetime::Transient;
		IndexBufferDesc.Usage = ERHIBufferUsage::Index;
		IndexBufferDesc.Memory = ERHIResourceMemory::HostVisible;
		IndexBuffer.reset(GetRHI()->RHICreateBuffer(IndexBufferDesc));

		RRHISamplerDesc SamplerDesc = {};
		SamplerDesc.MinFilter = ERHIFilterMode::Linear;
		SamplerDesc.MagFilter = ERHIFilterMode::Linear;
		SamplerDesc.MipFilter = ERHIFilterMode::Linear;
		SamplerDesc.AddressU = ERHIAddressMode::ClampToEdge;
		SamplerDesc.AddressV = ERHIAddressMode::ClampToEdge;
		SamplerDesc.AddressW = ERHIAddressMode::ClampToEdge;
		SamplerDesc.MinLod = 0.0f;
		SamplerDesc.MaxLod = 0.0f;

		size_t VertexBufferOffset = 0;
		size_t IndexBufferOffset = 0;

		std::vector<size_t> VertexBaseOffset(DrawData->CmdListsCount);
		std::vector<size_t> IndexBaseOffset(DrawData->CmdListsCount);

		for (int Index = 0; Index < DrawData->CmdListsCount; ++Index)
		{
			const ImDrawList* DrawList = DrawData->CmdLists[Index];
			const size_t VertexBufferSize = DrawList->VtxBuffer.Size * sizeof(ImDrawVert);
			const size_t IndexBufferSize = DrawList->IdxBuffer.Size * sizeof(ImDrawIdx);
			
			VertexBuffer->Map(DrawList->VtxBuffer.Data, VertexBufferSize, VertexBufferOffset);
			IndexBuffer->Map(DrawList->IdxBuffer.Data, IndexBufferSize, IndexBufferOffset);

			VertexBaseOffset[Index] = VertexBufferOffset / sizeof(ImDrawVert);
			IndexBaseOffset[Index] = IndexBufferOffset / sizeof(ImDrawIdx);
			
			VertexBufferOffset += VertexBufferSize; 
			IndexBufferOffset += IndexBufferSize;
		}

		const glm::vec2 Scale = { 2.0f / DrawData->DisplaySize.x, 2.0f / DrawData->DisplaySize.y };
		const glm::vec2 Translate = { -1.0f - DrawData->DisplayPos.x * Scale.x, -1.0f - DrawData->DisplayPos.y * Scale.y };

		for (int Index = 0; Index < DrawData->CmdListsCount; ++Index)
		{
			const ImDrawList* CmdList = DrawData->CmdLists[Index];
			size_t VertexBase = VertexBaseOffset[Index];
			size_t IndexBase = IndexBaseOffset[Index];

			for (const ImDrawCmd& CommandBuffer : CmdList->CmdBuffer)
			{
				FRHITexture* UserTexture = reinterpret_cast<FRHITexture*>(CommandBuffer.GetTexID());

				FRHITextureViewDesc TextureViewDesc;
				TextureViewDesc.Format = UserTexture->RHIGetTextureDesc().Format;
				TextureViewDesc.Aspect = ERHITextureAspect::Color;
				TextureViewDesc.Lifetime = ERHIResourceLifetime::Transient;

				TextureView.reset(GetRHI()->RHICreateTextureView(TextureViewDesc, UserTexture));
				Sampler.reset(GetRHI()->RHICreateSampler(SamplerDesc));
				
				glm::vec2 ClipMin = glm::vec2(Floor((CommandBuffer.ClipRect.x - DrawData->DisplayPos.x) * DrawData->FramebufferScale.x), Floor((CommandBuffer.ClipRect.y - DrawData->DisplayPos.y) * DrawData->FramebufferScale.y));
				glm::vec2 ClipMax = glm::vec2(Floor((CommandBuffer.ClipRect.z - DrawData->DisplayPos.x) * DrawData->FramebufferScale.x), Floor((CommandBuffer.ClipRect.w - DrawData->DisplayPos.y) * DrawData->FramebufferScale.y));
				if (ClipMax.x <= ClipMin.x || ClipMax.y <= ClipMin.y) 
				{ 
					IndexBase += CommandBuffer.ElemCount; continue; 
				}

				GetRHI()->RHICmdSetScissor(RHICmdList, (int32_t)ClipMin.x, (int32_t)ClipMin.y, (uint32_t)(ClipMax.x - ClipMin.x), (uint32_t)(ClipMax.y - ClipMin.y));

				FRHIDrawCommand DrawCommand = {};
				DrawCommand.IndexCount = CommandBuffer.ElemCount;
				DrawCommand.IndexBuffer = IndexBuffer.get();
				DrawCommand.VertexBuffer = VertexBuffer.get();
				DrawCommand.IndexBufferOffset = CommandBuffer.IdxOffset + IndexBaseOffset[Index];
				DrawCommand.VertexBufferOffset = CommandBuffer.VtxOffset + VertexBaseOffset[Index];
				DrawCommand.IndexType = FRHIDrawCommand::EIndexType::U16;

				FRHIDrawParams DrawParams = {};
				DrawParams.DrawParam_U32_1 = TextureView->GetBindingIndex();
				DrawParams.DrawParam_U32_2 = Sampler->GetBindingIndex();
				DrawParams.DrawParam_F32_1 = Scale[0];
				DrawParams.DrawParam_F32_2 = Scale[1];
				DrawParams.DrawParam_F32_3 = Translate[0];
				DrawParams.DrawParam_F32_4 = Translate[1];

				GetRHI()->RHICmdDrawIndexed(RHICmdList, DrawCommand, DrawParams);

				IndexBase += CommandBuffer.ElemCount;
			}
		}
	});
	UIPass.AddColorAttachment(EditorViewportRenderTarget.GetHandle());

	// These lines should be covered by the Execute function itself. Should not have to specify the composition and present passes manually.
	FFrameGraphPass& CompositionPass = Ctx.Builder.AddPass("Composition", EFGPassFlags::Compute, 
		[&](FRHICommandList& RHICmdList)
		{
			GetRHI()->RHICmdBlitImage(RHICmdList, EditorViewportRenderTarget.GetHandle(), GetRHI()->RHIGetBackBufferRenderTarget());
		});

	CompositionPass.AddTransferSrc(EditorViewportRenderTarget.GetHandle());
	CompositionPass.AddTransferDst(GetRHI()->RHIGetBackBufferRenderTarget());

	FFrameGraphPass& PresentPass = Ctx.Builder.AddPass("Present", EFGPassFlags::Graphics, nullptr);
	PresentPass.AddPresentSrc(GetRHI()->RHIGetBackBufferRenderTarget());

	// This should be a unified call done by FrameGraph itself, not the builder.
	// Should take all the builder's and combine them. (eg. Editor, Game)
	Ctx.Builder.Execute();
}