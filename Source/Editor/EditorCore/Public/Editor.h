#pragma once

#include "EditorBase.h"

class RModule;
class REditorRenderer;

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

    REditorRenderer* Renderer;

    RModule* GameModule;
};
