#pragma once

class IPanel
{
public:
    virtual ~IPanel() = default;

    virtual const char* GetName() const = 0;
    virtual void Draw(float DeltaTime) = 0;
};