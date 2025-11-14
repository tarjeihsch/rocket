#pragma once

#include "EditorBase.h"

class RModule;
class RViewport;
class REditorViewportClient;
class RLevelEditorViewportClient;
class RWorld;
class RSelection;

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
    virtual RWorld* GetEditorWorld() const override;
    virtual RSelection* GetSelection() const override;

private:
    EMode Mode;

    RWorld* EditorWorld;

    RModule* ScriptClientModule;

    RViewport* Viewport;

    REditorViewportClient* EditorViewportClient;

    RLevelEditorViewportClient* LevelEditorViewportClient;

    RSelection* Selection;
};
