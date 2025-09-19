#include "SceneRenderer.h"

#include "../../../ThirdParty/imgui/imgui.h"
#include "../../Engine/Public/Engine.h"
#include "../../Window/Public/Input.h"
#include "../../Window/Public/Window.h"
#include "Attachment.h"
#include "Camera.h"
#include "Common/RHI.h"
#include "Common/RHIBuffer.h"
#include "Common/RHICommandList.h"
#include "Common/RHIDrawCommand.h"
#include "Common/RHISampler.h"
#include "FrameGraph.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "StaticMesh.h"
#include "Transform.h"
#include "glm/gtc/matrix_inverse.hpp"

FRHITexture* Texture;

void FSceneRenderer::Init()
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)1280, (float)720);

    unsigned char* pixels;
    int width, height;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    Texture = GetRHI()->RHICreateTexture(FRHITextureDesc::Create2D(width, height, ERHIResourceLifetime::Persistent, ERHITextureFormat::R8G8B8A8_UNORM, ERHITextureUsage::SRV | ERHITextureUsage::CopyDst, ERHITextureLayout::Undefined, 0, 1, 0));

    GetRHI()->RHIUploadTextureBlocking(Texture, pixels, width, height);
}
static inline float  ImFloor(float f)                                           { return (float)((f >= 0 || (float)(int)f == f) ? (int)f : (int)f - 1); } // Decent replacement for floorf()

void FSceneRenderer::Tick()
{
    // Traverse the scene
    // Gather all Static Mesh Components
    // Add each ID from SMC to an array of FMeshDrawCommand (eg. DrawCommands)
    // At the end, call RenderGraph.AddPass([&](FRHICommandList& CmdList)
    // {
    //      CmdList.Enqueue<FRHIDraw>(DrawCommands)
    // });

    // Apply the same concept for Instanced Static Mesh Components, same for IndexedStaticMeshComponents and IndexedIndirectStaticMeshComponent

  // FPipeline Pipeline = FPipeline(ROCKET_SRC "/Shader/VertexShader.hlsl", ROCKET_SRC "/Shader/FragmentShader.hlsl");
  FRenderTarget RenderTarget = FRenderTarget(GetRHI()->RHIGetViewportExtent()[0], GetRHI()->RHIGetViewportExtent()[1]);
  // FAttachment DepthAttachment = FAttachment(GetRHI()->RHIGetViewportExtent()[0], GetRHI()->RHIGetViewportExtent()[1], ERHITextureFormat::D32_SFLOAT_S8_UINT, ERHITextureUsage::Depth | ERHITextureUsage::Stencil);

  FFrameGraphBuilder Builder;

  // FFrameGraphPass& DrawPass = Builder.AddPass("Draw Static Mesh", EFGPassFlags::Graphics | EFGPassFlags::Dynamic, [&](FRHICommandList& RHICmdList)
  // {
  //     GetRHI()->RHICmdBindPipeline(RHICmdList, Pipeline.GetHandle());

  //     if (!Camera)
  //     {
  //         return;
  //     }

  //     FRHIBufferMemoryPacker CameraData(ERHIBufferMemoryPackerAlignmentLayout::STD430);
  //     CameraData.Write(Camera->GetCachedCapture().ViewMatrix);
  //     CameraData.Write(Camera->GetCachedCapture().Projection);
  //     std::shared_ptr<FRHIBuffer> CameraInterface_GPUBuffer = GetRHI()->RHIBindBuffer(CameraData.GetData(), CameraData.GetSize());

  //     for (auto Proxy : Proxies)
  //     {
  //         CStaticMesh* StaticMesh = Proxy.StaticMesh;
  //         FTransform* Transform = Proxy.Transform;

  //         for (const auto& Section : StaticMesh->GetMeshSections())
  //         {
  //             const glm::mat4 ModelMatrix = Transform->ToMatrix() * Section.M_LocalMatrix;

  //             FRHIBufferMemoryPacker ObjectData(ERHIBufferMemoryPackerAlignmentLayout::STD430);
  //             ObjectData.Write(ModelMatrix);
  //             ObjectData.Write(glm::inverseTranspose(ModelMatrix));
  //             std::shared_ptr<FRHIBuffer> ObjectInterface_GPUBuffer = GetRHI()->RHIBindBuffer(ObjectData.GetData(), ObjectData.GetSize());

  //             FRHIDrawCommand DrawCommand = {};
  //             DrawCommand.VertexBuffer = StaticMesh->GetVertexBuffer();
  //             DrawCommand.IndexBuffer = StaticMesh->GetIndexBuffer();
  //             DrawCommand.IndexCount = Section.M_IndexCount;
  //             DrawCommand.VertexBufferOffset = Section.M_VertexOffset;
  //             DrawCommand.IndexBufferOffset = Section.M_IndexOffset;

  //             FRHIDrawParams DrawParams = {};
  //             DrawParams.DrawParam_1 = CameraInterface_GPUBuffer->GetBindingIndex(); // Camera ID
  //             DrawParams.DrawParam_2 = ObjectInterface_GPUBuffer->GetBindingIndex(); // Instance ID

  //             GetRHI()->RHICmdDrawIndexed(RHICmdList, DrawCommand, DrawParams);
  //         }
  //     }
  // });
  // DrawPass.AddColorAttachment(RenderTarget.GetHandle());
  // DrawPass.AddDepthAttachment(DepthAttachment.GetHandle());

    static bool a = true;

    std::unique_ptr<FRHIBuffer> VertexBuffer;
    std::unique_ptr<FRHIBuffer> IndexBuffer;
    std::unique_ptr<FRHITextureView> TextureView;
    std::unique_ptr<RRHISampler> Sampler;
    ImGuiIO& io = ImGui::GetIO();
    const auto extent = GetRHI()->RHIGetViewportExtent(); // should be client size (w,h)
    io.DisplaySize = ImVec2((float)extent[0], (float)extent[1]); // client area ONLY
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);             // or your DPI scale
    // If for some reason your render area starts at an offset inside a larger image:
    //io.Posi = ImVec2(0.0f, 0.0f); // make sure it's 0 unless you really need an offset

    // Timing
    io.DeltaTime = GetEngine()->GetTimestep().GetDeltaTime();   // must be >0


    // Mouse
    io.MousePos = ImVec2((float)PInput::GetMouseX(), (float)PInput::GetMouseY());


    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0, 50), ImGuiCond_Always);

    ImGui::ShowDemoWindow(&a);
    ImGui::Render();
    ImDrawData* DrawData = ImGui::GetDrawData();

    FRHIPipelineDesc ImGuiPipelineDesc = {};

    FPipeline ImGuiPipeline = FPipeline(FRHIPipelineDesc{ROCKET_SRC "/Shader/ImGuiVS.hlsl", ROCKET_SRC "/Shader/ImGuiPS.hlsl"});

    FFrameGraphPass& ImGuiPass = Builder.AddPass("ImGui", EFGPassFlags::Graphics | EFGPassFlags::Dynamic, [&](FRHICommandList& RHICmdList)
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
        VertexBuffer.reset (GetRHI()->RHICreateBuffer(VertexBufferDesc));

        FRHIBufferDesc IndexBufferDesc = { ibBytes, ERHIResourceLifetime::Transient, ERHIBufferUsage::Index, ERHIResourceMemory::HostVisible };
        IndexBuffer.reset (GetRHI()->RHICreateBuffer(IndexBufferDesc));

        RRHISamplerDesc desc{};
        desc.MinFilter = ERHIFilterMode::Linear;
        desc.MagFilter = ERHIFilterMode::Linear;
        desc.MipFilter = ERHIFilterMode::Linear;
        desc.AddressU  = ERHIAddressMode::ClampToBorder;
        desc.AddressV  = ERHIAddressMode::ClampToBorder;
        desc.AddressW  = ERHIAddressMode::ClampToBorder;
        desc.MinLod    = 0.0f;
        desc.MaxLod    = 0.0f;   // no mipmaps

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
        pc.scale.x     =  2.0f / DrawData->DisplaySize.x;
        pc.scale.y     =  2.0f / DrawData->DisplaySize.y;
        pc.translate.x = -1.0f - DrawData->DisplayPos.x * pc.scale.x;
        pc.translate.y = -1.0f - DrawData->DisplayPos.y * pc.scale.y;


        for (int n = 0; n < DrawData->CmdListsCount; ++n)
        {
            const ImDrawList* cl = DrawData->CmdLists[n];
            size_t idxOffset = listIdxBase[n];
            size_t vtxBase   = listVtxBase[n];

            for (const ImDrawCmd& cmd : cl->CmdBuffer)
            {
                if (cmd.UserCallback) { cmd.UserCallback(cl, &cmd); continue; }

                FRHITextureViewDesc TextureViewDesc;
                TextureViewDesc.Format = ERHITextureFormat::R8G8B8A8_UNORM;
                TextureViewDesc.Aspect = ERHITextureAspect::Color;
                TextureViewDesc.Lifetime = ERHIResourceLifetime::Transient;
                TextureView.reset (GetRHI()->RHICreateTextureView(TextureViewDesc, Texture));
                Sampler.reset (GetRHI()->RHICreateSampler(desc));

                ImVec2 clipMin = (ImVec2(ImFloor((cmd.ClipRect.x - DrawData->DisplayPos.x) * DrawData->FramebufferScale.x), ImFloor((cmd.ClipRect.y - DrawData->DisplayPos.y) * DrawData->FramebufferScale.y)));
                ImVec2 clipMax = (ImVec2(ImFloor((cmd.ClipRect.z - DrawData->DisplayPos.x) * DrawData->FramebufferScale.x), ImFloor((cmd.ClipRect.w - DrawData->DisplayPos.y) * DrawData->FramebufferScale.y)));
                if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y) { idxOffset += cmd.ElemCount; continue; }

                GetRHI()->RHICmdSetScissor(RHICmdList, (int32_t)clipMin.x, (int32_t)clipMin.y, (uint32_t)(clipMax.x - clipMin.x), (uint32_t)(clipMax.y - clipMin.y));

                FRHIDrawCommand dc = {};
                dc.IndexCount        = cmd.ElemCount;
                dc.IndexBuffer       = IndexBuffer.get();
                dc.VertexBuffer      = VertexBuffer.get();
                dc.IndexBufferOffset     = cmd.IdxOffset + listIdxBase[n];  // in indices
                dc.VertexBufferOffset   = cmd.VtxOffset + listVtxBase[n];  // in vertices

                FRHIDrawParams dp = {};
                dp.DrawParam_1 = pc.scale[0];
                dp.DrawParam_2 = pc.scale[1];
                dp.DrawParam_3 = pc.translate[0];
                dp.DrawParam_4 = pc.translate[1];
                dp.DrawParam_5 = TextureView->GetBindingIndex();
                dp.DrawParam_6 = Sampler->GetBindingIndex();

                GetRHI()->RHICmdDrawIndexed(RHICmdList, dc, dp);

                idxOffset += cmd.ElemCount;
            }
        }
    });
    ImGuiPass.AddColorAttachment(RenderTarget.GetHandle());

    FFrameGraphPass& CompositionPass = Builder.AddPass("Composition", EFGPassFlags::Compute, [&](FRHICommandList& RHICmdList)
    {
        GetRHI()->RHICmdBlitImage(RHICmdList, RenderTarget.GetHandle(), GetRHI()->RHIGetBackBufferRenderTarget());
    });
    CompositionPass.AddTransferSrc(RenderTarget.GetHandle());
    CompositionPass.AddTransferDst(GetRHI()->RHIGetBackBufferRenderTarget());

    FFrameGraphPass& PresentPass = Builder.AddPass("Present", EFGPassFlags::Graphics, nullptr);
    PresentPass.AddPresentSrc(GetRHI()->RHIGetBackBufferRenderTarget());

    Builder.Execute();
}

void FSceneRenderer::RegisterProxy(CStaticMesh *StaticMesh, FTransform* Transform)
{
    RRenderProxy Proxy = { StaticMesh, Transform};

    Proxies.push_back(Proxy);
}
void FSceneRenderer::SetCamera(ICamera *InCamera)
{
    Camera = InCamera;

    RK_LOG_INFO("asdasdasd");
}
