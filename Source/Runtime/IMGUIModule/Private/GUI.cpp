#include "GUI.h"

RGUI::RGUI()
    : Name("GUI")
{
}

RGUI::RGUI(const char* InName)
    : Name(InName),
    VisibilityMode(EGUIVisibilityMode::Visible)
{
}

void RGUI::Update(float DeltaTime)
{
    if (VisibilityMode > EGUIVisibilityMode::Visible)
    {
        return;
    }
}

void RGUI::AttachWidget(const std::shared_ptr<RGUI>& ChildWidget)
{
    Children.emplace_back(ChildWidget);
}

void RGUI::DetachWidget(const std::shared_ptr<RGUI>& ChildWidget)
{
}

