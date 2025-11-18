#include "Dockspace.h"

#include "imgui.h"

int32_t DockspaceFlags = 0;
bool bDockspaceOpen = true;
bool bFullscreen = true;
bool bPadding = false;

RDockspace::RDockspace()
    : RWidget("Dockspace")
{
    WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
}

RDockspace::RDockspace(const char* Name)
    : RWidget(Name)
{
    WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
}

void RDockspace::PreUpdate()
{
    ImGuiIO& IO = ImGui::GetIO();

    const ImGuiViewport* MainViewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(MainViewport->WorkPos);
    ImGui::SetNextWindowSize(MainViewport->WorkSize);
    ImGui::SetNextWindowViewport(MainViewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
}

void RDockspace::UpdateImpl()
{
    ImGuiIO& IO = ImGui::GetIO();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    if (IO.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID DockspaceID = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(DockspaceID, ImVec2(0.0f, 0.0f), DockspaceFlags);
    }
}