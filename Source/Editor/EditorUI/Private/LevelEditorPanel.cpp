#include "LevelEditorPanel.h"

#include "EngineBase.h"
#include "Renderer.h"

#include <imgui.h>

RLevelEditorPanel::RLevelEditorPanel()
    : RPanel("Level Editor")
{
}

void RLevelEditorPanel::UpdatePanelImpl()
{
    //ImGui::Image(GEngine->GetRenderer()->GetColorRenderTarget(), ImGui::GetContentRegionAvail());
}