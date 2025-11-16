#pragma once

#include "EngineBase.h"

#include <memory>

class REditorUI;
class RWorld;
class RRenderer;

class REditorEngine final : public IEngineBase
{
public:
    enum class EMode
    {
        Edit, Play, Pause, Step
    };

    REditorEngine();

    virtual void Start(REngineInitParams& InitParams) override;
    virtual void Tick(float DeltaTime) override;
    virtual void Stop() override;
    virtual void LoadWorld(const char* Path) override;
    virtual void UnloadWorld() override;
    virtual RWorld* GetWorld() const override;
    virtual RGame* GetGame() const override;
    virtual RRenderer* GetRenderer() const override;

    void EnterPlayMode();
    void ExitPlayMode();
    void PauseFrame();
    void StepFrame();

private:
    // Editable world - copied to game world on play
    std::unique_ptr<RWorld> World;

    std::unique_ptr<RRenderer> Renderer;

    std::unique_ptr<REditorUI> UI;

    // Current editor mode
    EMode Mode;
};
