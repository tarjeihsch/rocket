#include "Widget.h"

#include "Assertion.h"
#include "EngineBase.h"
#include "Material.h"
#include "MathTypes.h"
#include "Primitive.h"
#include "Renderer.h"
#include "Sampler.h"
#include "Viewport.h"
#include "Common/RHIBuffer.h"
#include "Common/RHIDrawCommand.h"
#include "glm/vec2.hpp"

#include <imgui.h>

RWidget::RWidget()
    : WidgetName("Root"),
    Visibility(EWidgetVisiblityState::Visible),
    WindowFlags(0)
{
}

RWidget::RWidget(const char* InWidgetName)
    : WidgetName(InWidgetName),
    Visibility(EWidgetVisiblityState::Visible),
    WindowFlags(0)
{
}

void RWidget::Update(float DeltaTime)
{
    if (Visibility != EWidgetVisiblityState::Visible)
    {
        return;
    }

    PreUpdate();

    bool IsOpen = true;
    ImGui::Begin(WidgetName, &IsOpen, WindowFlags);

    const ImDrawList* DrawList = ImGui::GetWindowDrawList();
    ensure(DrawList != nullptr);
    UpdateImpl();

    ImGui::End();

    UpdatePanelDeferredImpl(*DrawList);

    Visibility = IsOpen ? EWidgetVisiblityState::Visible : EWidgetVisiblityState::Hidden;

    for (const std::shared_ptr<RWidget>& Child : Children)
    {
        Child->Update(DeltaTime);
    }
}

void RWidget::Render(float DeltaTime)
{
    GEngine->GetRenderer()->DrawIndexed(GEngine->GetViewport(), DrawCall);

    for (const std::shared_ptr<RWidget>& Child : Children)
    {
        Child->Render(DeltaTime);
    }
}

void RWidget::AttachWidget(const std::shared_ptr<RWidget>& ChildWidget)
{
    Children.emplace_back(ChildWidget);
}

void RWidget::UpdatePanelDeferredImpl(const ImDrawList& DrawList)
{
    ImGuiIO& IO = ImGui::GetIO();

    glm::vec2 Scale = {};
    Scale.x = 2.0f / IO.DisplaySize.x;
    Scale.y = 2.0f / IO.DisplaySize.y;

    glm::vec2 Translate = {};
    Translate.x = -1.0f - 0.0f * Scale.x;
    Translate.y = -1.0f - 0.0f * Scale.y;

    FRHIBufferMemoryPacker ProjectionBufferData = FRHIBufferMemoryPacker(ERHIBufferMemoryPackerAlignmentLayout::STD430);
    ProjectionBufferData.Write(Scale);
    ProjectionBufferData.Write(Translate);
    std::shared_ptr<FRHIBuffer> ProjectionBuffer = GetRHI()->RHIBindBuffer(ProjectionBufferData.GetData(), ProjectionBufferData.GetSize());

    for (const ImDrawCmd& DrawCmd : DrawList.CmdBuffer)
    {
        if (DrawCmd.ElemCount == 0)
        {
            continue;
        }

        FRHIBuffer* VertexBuffer = GetRHI()->RHICreateBuffer(FRHIBufferDesc::CreateTransientVertexBuffer(DrawCmd.ElemCount * sizeof(ImDrawVert)));
        VertexBuffer->Map(&DrawList.VtxBuffer[static_cast<int32_t>(DrawCmd.VtxOffset)], VertexBuffer->GetBufferDesc().Size);

        FRHIBuffer* IndexBuffer = GetRHI()->RHICreateBuffer(FRHIBufferDesc::CreateTransientIndexBuffer(DrawCmd.ElemCount * sizeof(ImDrawIdx)));
        IndexBuffer->Map(&DrawList.IdxBuffer[static_cast<int32_t>(DrawCmd.IdxOffset)], IndexBuffer->GetBufferDesc().Size);

        FRHITexture* Texture = reinterpret_cast<FRHITexture*>(DrawCmd.GetTexID());
        FRHITextureViewDesc UserTextureViewDesc = {};
        UserTextureViewDesc.Format = Texture->RHIGetTextureDesc().Format;
        UserTextureViewDesc.Aspect = ERHITextureAspect::Color;
        UserTextureViewDesc.Lifetime = ERHIResourceLifetime::Transient;

        std::shared_ptr<RPrimitive> Primitive = std::make_shared<RPrimitive>(VertexBuffer, IndexBuffer);
        std::shared_ptr<RMaterial> Material = std::make_shared<RMaterial>(EMaterialPass::UI, EMaterialBlendMode::Transparent, ROCKET_SRC "/Shader/ImGuiVS.hlsl", ROCKET_SRC "/Shader/ImGuiPS.hlsl");
        std::shared_ptr<RSampler> Sampler = std::make_shared<RSampler>();
        std::shared_ptr<FRHITextureView> UserTextureView(GetRHI()->RHICreateTextureView(UserTextureViewDesc, Texture));

        float ClipMinX = Floor(DrawCmd.ClipRect.x * IO.DisplayFramebufferScale.x);
        float ClipMinY = Floor(DrawCmd.ClipRect.y * IO.DisplayFramebufferScale.y);
        float ClipMaxX = Floor(DrawCmd.ClipRect.z * IO.DisplayFramebufferScale.x);
        float ClipMaxY = Floor(DrawCmd.ClipRect.w * IO.DisplayFramebufferScale.y);

        std::array<float, 2> ScissorPosition = { ClipMinX, ClipMinY };
        std::array<float, 2> ScissorExtent = { ClipMaxX - ClipMinX, ClipMaxY - ClipMinY};

        DrawCall = {};
        DrawCall.Primitive = Primitive;
        DrawCall.Material = Material;
        DrawCall.ScissorPosition = ScissorPosition;
        DrawCall.ScissorExtent = ScissorExtent;
        DrawCall.IndexType = FRHIDrawCall::EIndexType::U16;
        DrawCall.Params.DrawParam_U32_1 = UserTextureView->GetBindingIndex();
        DrawCall.Params.DrawParam_U32_2 = Sampler->GetBindingIndex();
        DrawCall.Params.DrawParam_U32_3 = ProjectionBuffer->GetBindingIndex();
    }
}