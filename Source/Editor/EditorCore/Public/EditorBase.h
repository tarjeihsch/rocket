#pragma once

class RWorld;
class RSelection;

class IEditorBase
{
public:
    enum class EMode
    {
        Edit, Play, Pause, Step
    };

    virtual ~IEditorBase() = default;

    virtual void Init() = 0;
    virtual void Tick(float DeltaTime) = 0;
    virtual void Shutdown() = 0;
    virtual void EnterPlayMode() = 0;
    virtual void ExitPlayMode() = 0;
    virtual void PauseFrame() = 0;
    virtual void StepFrame() = 0;
    virtual EMode GetMode() const = 0;
    virtual RWorld* GetEditorWorld() const = 0;
    virtual RSelection* GetSelection() const = 0;

private:
    EMode Mode;
};

extern IEditorBase* GEditor;