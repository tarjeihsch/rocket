//#include "../Public/SceneRenderer.h"
//
//#include "../../../ThirdParty/imgui/imgui.h"
//#include "../../Engine/Public/EngineBase.h"
//#include "../../Engine/Public/LocalPlayer.h"
//#include "Attachment.h"
//#include "Common/RHI.h"
//#include "Common/RHIBuffer.h"
//#include "Common/RHICommandList.h"
//#include "Common/RHIDrawCommand.h"
//#include "Common/RHISampler.h"
//#include "FrameGraph.h"
//#include "Input.h"
//#include "KeyCode.h"
//#include "Pipeline.h"
//#include "RenderTarget.h"
//#include "PrimitiveManager.h"
//#include "StaticMesh.h"
//#include "Transform.h"
//#include "Viewport.h"
//#include "glm/gtc/matrix_inverse.hpp"
//
//std::unique_ptr<FRHITexture> Texture;
//
////void RSceneRenderer::Init()\
////{
////    ImGui::CreateContext();
////    ImGuiIO& io = ImGui::GetIO();
////    io.DisplaySize = ImVec2((float)1280, (float)720);
////
////    unsigned char* pixels;
////    int width, height;
////    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
////
////    Texture.reset(GetRHI()->RHICreateTexture(FRHITextureDesc::Create2D(width, height, ERHIResourceLifetime::Persistent, ERHITextureFormat::R8G8B8A8_UNORM, ERHITextureUsage::SRV | ERHITextureUsage::CopyDst, ERHITextureLayout::Undefined, 0, 1, 0)));
////
////    GetRHI()->RHIUploadTextureBlocking(Texture.get(), pixels, width, height);
////}
//
//static inline float ImFloor(float f) { return (float)((f >= 0 || (float)(int)f == f) ? (int)f : (int)f - 1); } // Decent replacement for floorf()
//
//void RSceneRenderer::Render(RViewport* Viewport)
//{
//    FFrameGraphBuilder Builder;
//
//    FRenderTarget RenderTarget = FRenderTarget(GetRHI()->RHIGetViewportExtent()[0], GetRHI()->RHIGetViewportExtent()[1]);
//
//    FRHIPipelineDesc PipelineSpec = {};
//    PipelineSpec.PixelShaderPath = ROCKET_SRC "/Shader/PixelShader.hlsl";
//    PipelineSpec.VertexShaderPath = ROCKET_SRC "/Shader/VertexShader.hlsl";
//    PipelineSpec.bDepthTestEnable = true;
//    PipelineSpec.bDepthWriteEnable = true;
//    PipelineSpec.DepthFormat = ERHIFormat::D32_Float_S8X24;
//    PipelineSpec.CullMode = ERHICullMode::Back;
//    PipelineSpec.ColorAttachmentFormats.emplace_back(ERHIFormat::BGRA8_SRGB);
//    PipelineSpec.VertexBindings.emplace_back(FRHIVertexBinding(0, 48, 0));
//    PipelineSpec.VertexAttributes.emplace_back(0, 0, ERHIFormat::Float3, offsetof(FRHIVertex, m_Position));
//    PipelineSpec.VertexAttributes.emplace_back(1, 0, ERHIFormat::Float3, offsetof(FRHIVertex, m_Normal));
//    PipelineSpec.VertexAttributes.emplace_back(2, 0, ERHIFormat::Float2, offsetof(FRHIVertex, m_TexCoord));
//    PipelineSpec.BlendAttachments.emplace_back(FRHIBlendAttachment(false, ERHIBlendFactor::SrcAlpha, ERHIBlendFactor::OneMinusSrcAlpha, ERHIBlendOp::Add, ERHIBlendFactor::One, ERHIBlendFactor::Zero, ERHIBlendOp::Add, (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3)));
//
//    FRHIPipeline* Pipeline = GetRHI()->RHICreatePipelineStateObject(PipelineSpec);
//    FAttachment DepthAttachment = FAttachment(GetRHI()->RHIGetViewportExtent()[0], GetRHI()->RHIGetViewportExtent()[1], ERHITextureFormat::D32_SFLOAT_S8_UINT, ERHITextureUsage::Depth | ERHITextureUsage::Stencil);
//
//    FFrameGraphPass& DrawPass = Builder.AddPass("Draw Static Mesh", EFGPassFlags::Graphics | EFGPassFlags::Dynamic | EFGPassFlags::ClearColor, [&](FRHICommandList& RHICmdList)
//    {
//        GetRHI()->RHICmdBindPipeline(RHICmdList, Pipeline);
//
//        FRHIBufferMemoryPacker CameraData(ERHIBufferMemoryPackerAlignmentLayout::STD430);
//        CameraData.Write(Scene->GetViews()[0].ViewMatrix);
//        CameraData.Write(Scene->GetViews()[0].ProjectionMatrix);
//        std::shared_ptr<FRHIBuffer> CameraInterface_GPUBuffer = GetRHI()->RHIBindBuffer(CameraData.GetData(), CameraData.GetSize());
//
//        for (const LSceneProxy& Proxy : Scene->GetProxies())
//        {
//            RStaticMesh* StaticMesh = Proxy.Geometry.StaticMesh;
//            for (const auto& Section : StaticMesh->GetMeshSections())
//            {
//                FRHIBufferMemoryPacker ObjectData(ERHIBufferMemoryPackerAlignmentLayout::STD430);
//                ObjectData.Write(Proxy.Transform.ModelMatrix * Section.LocalMatrix);
//                ObjectData.Write(Proxy.Transform.InverseModelMatrix);
//                std::shared_ptr<FRHIBuffer> ObjectInterface_GPUBuffer = GetRHI()->RHIBindBuffer(ObjectData.GetData(), ObjectData.GetSize());
//
//                FRHIDrawCommand DrawCommand = {};
//                DrawCommand.VertexBuffer = StaticMesh->GetVertexBuffer();
//                DrawCommand.IndexBuffer = StaticMesh->GetIndexBuffer();
//                DrawCommand.IndexCount = Section.M_IndexCount;
//                DrawCommand.VertexBufferOffset = Section.M_VertexOffset;
//                DrawCommand.IndexBufferOffset = Section.M_IndexOffset;
//                DrawCommand.IndexType = FRHIDrawCommand::EIndexType::U32;
//
//                FRHIDrawParams DrawParams = {};
//                DrawParams.DrawParam_U32_1 = CameraInterface_GPUBuffer->GetBindingIndex(); // Camera ID
//                DrawParams.DrawParam_U32_2 = ObjectInterface_GPUBuffer->GetBindingIndex(); // Instance ID
//
//                GetRHI()->RHICmdDrawIndexed(RHICmdList, DrawCommand, DrawParams);
//            }
//        }
//    });
//    DrawPass.AddColorAttachment(RenderTarget.GetHandle());
//    DrawPass.AddDepthAttachment(DepthAttachment.GetHandle());
//

//
//    FFrameGraphPass& CompositionPass = Builder.AddPass("Composition", EFGPassFlags::Compute, [&](FRHICommandList& RHICmdList)
//    {
//        GetRHI()->RHICmdBlitImage(RHICmdList, RenderTarget.GetHandle(), GetRHI()->RHIGetBackBufferRenderTarget());
//    });
//    CompositionPass.AddTransferSrc(RenderTarget.GetHandle());
//    CompositionPass.AddTransferDst(GetRHI()->RHIGetBackBufferRenderTarget());
//
//    FFrameGraphPass& PresentPass = Builder.AddPass("Present", EFGPassFlags::Graphics, nullptr);
//    PresentPass.AddPresentSrc(GetRHI()->RHIGetBackBufferRenderTarget());
//
//    Builder.Execute();
//}
//
//void RSceneRenderer::Shutdown()
//{
//}
