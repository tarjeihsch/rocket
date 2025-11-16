#include "EditorUI.h"

#include "EngineBase.h"
#include "Input.h"
#include "KeyCode.h"
#include "LevelEditorPanel.h"
#include "Panel.h"
#include "Renderer.h"
#include "Texture2D.h"

#include <imgui.h>
#include <Common/RHIBuffer.h>
#include "Primitive.h"
#include <MathTypes.h>
#include <Material.h>
#include <Sampler.h>

REditorUI::REditorUI()
{
    Initialize();
}

REditorUI::~REditorUI()
{
}

void REditorUI::Initialize()
{
    ImGui::CreateContext();
    ImGuiIO& IO = ImGui::GetIO();
    IO.DisplaySize = ImVec2(static_cast<float>(1280), static_cast<float>(720));
    IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    unsigned char* Data;
    int Width, Height;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&Data, &Width, &Height);

    FontTex2D = std::make_unique<RTexture2D>(ETextureType::Linear, Width, Height);
    FontTex2D->LoadRawTextureData(Data);
    IO.Fonts->SetTexID((ImTextureID)FontTex2D->GetHandle());

    AddPanel(new RLevelEditorPanel());
}

void REditorUI::AddPanel(RPanel* Panel)
{
    const std::shared_ptr<RPanel> Ref(Panel);

    Panels.push_back(Ref);
}

void REditorUI::Update(float DeltaTime)
{
    ImGuiIO& IO = ImGui::GetIO();
    IO.DisplaySize = ImVec2(GetRHI()->RHIGetViewportExtent()[0], GetRHI()->RHIGetViewportExtent()[1]);
    IO.DeltaTime = DeltaTime;
    IO.MousePos = ImVec2((float)PInput::GetMouseX(), (float)PInput::GetMouseY());
    IO.MouseDown[0] = PInput::GetMouseButtonPressed(RK_MOUSE_BUTTON_LEFT);
    IO.MouseDown[1] = PInput::GetMouseButtonPressed(RK_MOUSE_BUTTON_RIGHT);

	ImGui::NewFrame();

    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (bFullscreen)
    {
        const ImGuiViewport* MainViewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(MainViewport->WorkPos);
        ImGui::SetNextWindowSize(MainViewport->WorkSize);
        ImGui::SetNextWindowViewport(MainViewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    if (!bPadding)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }

    ImGui::Begin("MainDockspace", &bDockspaceOpen, WindowFlags);

    if (!bPadding)
    {
        ImGui::PopStyleVar();
    }

    if (bFullscreen)
    {
        ImGui::PopStyleVar(2);
    }

    if (IO.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID DockspaceID = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(DockspaceID, ImVec2(0.0f, 0.0f), DockspaceFlags);
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit"))
            {
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Object"))
        {
            if (ImGui::MenuItem("New Actor"))
            {
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    for (auto& Panel : Panels)
    {
        Panel->UpdatePanel();
    }

    ImGui::End();
    ImGui::Render();

    BuildDrawData();
    BuildDrawData();

}

void REditorUI::BuildDrawData()
{
    ImDrawData* DrawData = ImGui::GetDrawData();

    glm::vec2 Scale = {};
    Scale.x = 2.0f / DrawData->DisplaySize.x;
    Scale.y = 2.0f / DrawData->DisplaySize.y;

    glm::vec2 Translate = {};
    Translate.x = -1.0f - DrawData->DisplayPos.x * Scale.x;
    Translate.y = -1.0f - DrawData->DisplayPos.y * Scale.y;

    FRHIBufferMemoryPacker ProjectionBufferData = FRHIBufferMemoryPacker(ERHIBufferMemoryPackerAlignmentLayout::STD430);
    ProjectionBufferData.Write(Scale);
    ProjectionBufferData.Write(Translate);
    std::shared_ptr<FRHIBuffer> ProjectionBuffer = GetRHI()->RHIBindBuffer(ProjectionBufferData.GetData(), ProjectionBufferData.GetSize());

    std::vector<FRHIDrawCall> DrawCommands;

    for (int Index = 0; Index < DrawData->CmdListsCount; ++Index)
    {
        const ImDrawList* CmdList = DrawData->CmdLists[Index];

        for (const ImDrawCmd& CommandBuffer : CmdList->CmdBuffer)
        {
            // Each draw command corresponds to a draw call, so we need to extract the vertex and index data for this command.
            // Create a new vertex buffer for this draw call.
            size_t VertexBufferSize = CommandBuffer.ElemCount * sizeof(ImDrawVert);
            size_t IndexBufferSize = CommandBuffer.ElemCount * sizeof(ImDrawIdx);

            // Create a new vertex buffer and copy the relevant data from the ImDrawList's VtxBuffer
            FRHIBuffer* VertexBuffer = GetRHI()->RHICreateBuffer(FRHIBufferDesc::CreateTransientVertexBuffer(VertexBufferSize));
            VertexBuffer->Map(&CmdList->VtxBuffer[(int32_t)CommandBuffer.VtxOffset], VertexBufferSize);

            // Create a new index buffer and copy the relevant data from the ImDrawList's IdxBuffer
            FRHIBuffer* IndexBuffer = GetRHI()->RHICreateBuffer(FRHIBufferDesc::CreateTransientIndexBuffer(IndexBufferSize));
            IndexBuffer->Map(&CmdList->IdxBuffer[(int32_t)CommandBuffer.IdxOffset], IndexBufferSize);

            // Create the texture view for the current draw call's texture
            FRHITextureViewDesc UserTextureViewDesc = {};
            UserTextureViewDesc.Format = ERHITextureFormat::R8G8B8A8_UNORM;
            UserTextureViewDesc.Aspect = ERHITextureAspect::Color;
            UserTextureViewDesc.Lifetime = ERHIResourceLifetime::Transient;
            FRHITexture* UserTexture = reinterpret_cast<FRHITexture*>(CommandBuffer.GetTexID());
            std::shared_ptr<FRHITextureView> UserTextureView(GetRHI()->RHICreateTextureView(UserTextureViewDesc, UserTexture));

            // Create the primitive and material for the draw call
            std::shared_ptr<RPrimitive> Primitive = std::make_shared<RPrimitive>(VertexBuffer, IndexBuffer);
            std::shared_ptr<RMaterial> Material = std::make_shared<RMaterial>(EMaterialPass::UI, EMaterialBlendMode::Transparent, ROCKET_SRC "/Shader/ImGuiVS.hlsl", ROCKET_SRC "/Shader/ImGuiPS.hlsl");
            std::shared_ptr<RSampler> Sampler = std::make_shared<RSampler>();

            // Clip rect calculations
            float ClipMinX = Floor((CommandBuffer.ClipRect.x - DrawData->DisplayPos.x) * DrawData->FramebufferScale.x);
            float ClipMinY = Floor((CommandBuffer.ClipRect.y - DrawData->DisplayPos.y) * DrawData->FramebufferScale.y);
            float ClipMaxX = Floor((CommandBuffer.ClipRect.z - DrawData->DisplayPos.x) * DrawData->FramebufferScale.x);
            float ClipMaxY = Floor((CommandBuffer.ClipRect.w - DrawData->DisplayPos.y) * DrawData->FramebufferScale.y);

            std::array<float, 2> ScissorPosition = { ClipMinX, ClipMinY };
            std::array<float, 2> ScissorExtent = { ClipMaxX - ClipMinX, ClipMaxY - ClipMinY};

            // Setup the draw call with its parameters
            FRHIDrawCall DrawCommand;
            DrawCommand.Primitive = Primitive;
            DrawCommand.Material = Material;
            DrawCommand.ScissorPosition = ScissorPosition;
            DrawCommand.ScissorExtent = ScissorExtent;
            DrawCommand.IndexType = FRHIDrawCall::EIndexType::U16;
            DrawCommand.Params.DrawParam_U32_1 = UserTextureView->GetBindingIndex();
            DrawCommand.Params.DrawParam_U32_2 = Sampler->GetBindingIndex();
            DrawCommand.Params.DrawParam_U32_3 = ProjectionBuffer->GetBindingIndex();

            // Add the draw command to the list
            DrawCommands.push_back(DrawCommand);
        }
    }

    // Submit all the draw calls
    GEngine->GetRenderer()->DrawIndexed(DrawCommands);
}
