#pragma once

class FRHIPipeline;
class PScene;

class FSceneRenderer
{
public:
    void Init();
    void Tick(PScene* Scene);

private:
    FRHIPipeline* Pipeline;
};