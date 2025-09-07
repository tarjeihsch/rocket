#pragma once

class PScene;

class FSceneRenderer
{
public:
    void Init();
    void Tick(PScene* Scene);
};