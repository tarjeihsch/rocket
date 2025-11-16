#pragma once

#include <vector>
#include <memory>

struct RTexture2D;
class RPanel;

class REditorUI
{
public:
    REditorUI();
    ~REditorUI();

    void Initialize();
    void AddPanel(RPanel* Panel);
    void Update(float DeltaTime);

protected:
    void BuildDrawData();

protected:
    std::vector<std::shared_ptr<RPanel>> Panels;

    std::unique_ptr<RTexture2D> FontTex2D;

private:
    int32_t DockspaceFlags = 0;

    bool bDockspaceOpen = true;
    bool bFullscreen = true;
    bool bPadding = false;
};