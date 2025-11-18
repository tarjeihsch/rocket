#include "SceneHierarchyPanel.h"

#include "EngineBase.h"
#include "Actor.h"
#include "World.h"

#include <imgui.h>
#include <span>

RSceneHierarchyPanel::RSceneHierarchyPanel()
    : RWidget("Scene Hierarchy"),
    SelectedActor(nullptr)
{
}

void RSceneHierarchyPanel::UpdateImpl()
{
    std::span<RActor*> Actors = GEngine->GetWorld()->GetActors();
    for (size_t Index = 0; Index < Actors.size(); ++Index)
    {
        RActor* Actor = Actors[Index];
        std::string ActorLabel = "Actor_%u" + std::to_string(Actor->GetUUID());

        if (ImGui::Selectable(ActorLabel.c_str(), (SelectedActor == Actor)))
        {
            SelectedActor = Actor;
        }
    }
}