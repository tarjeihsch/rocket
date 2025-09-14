#pragma once

class CScene;

class FSceneRenderer
{
public:
    void Init();
    void Tick(CScene* Scene);
};