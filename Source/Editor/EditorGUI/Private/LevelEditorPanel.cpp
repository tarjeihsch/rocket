#include "LevelEditorPanel.h"

#include "EngineBase.h"
#include "RenderTarget.h"
#include "Renderer.h"

#include <imgui.h>

RLevelEditorPanel::RLevelEditorPanel()
    : RWidget("Level Editor")
{
}

void RLevelEditorPanel::UpdateImpl()
{
    //if (!GEngine->GetRenderer()->ColorRT) return;
    //ImGui::Image(GEngine->GetRenderer()->ColorRT->GetHandle(), ImGui::GetContentRegionAvail());
}