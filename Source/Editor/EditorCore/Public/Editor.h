#pragma once

#include "EditorBase.h"

class RModule;
class RViewport;
class REditorViewportClient;

class REditor final : public IEditorBase
{
public:
    REditor();

    virtual void Init() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Shutdown() override;
    virtual void EnterPlayMode() override;
    virtual void ExitPlayMode() override;
    virtual void PauseFrame() override;
    virtual void StepFrame() override;
    virtual EMode GetMode() const override;

private:
    EMode Mode;

    RModule* ScriptClientModule;

    RViewport* Viewport;

    REditorViewportClient* ViewportClient;
};
