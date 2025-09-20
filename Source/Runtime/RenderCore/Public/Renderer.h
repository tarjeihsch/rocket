#pragma once

class IRenderer
{
public:
    virtual ~IRenderer() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Tick(float DeltaTime) = 0;
};