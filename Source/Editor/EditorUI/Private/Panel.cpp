#include "Panel.h"

#include <imgui.h>

RPanel::RPanel(const char* InPanelName)
    : PanelName(InPanelName),
    Visibility(EPanelVisiblityState::Visible)
{
}

void RPanel::UpdatePanel()
{
    if (Visibility != EPanelVisiblityState::Visible)
    {
        return;
    }

    bool IsOpen = true;

    ImGui::Begin(PanelName, &IsOpen);
    UpdatePanelImpl();
    ImGui::End();

    Visibility = IsOpen ? EPanelVisiblityState::Hidden : EPanelVisiblityState::Visible;
}