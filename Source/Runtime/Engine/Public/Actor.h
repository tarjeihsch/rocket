#pragma once

#include <vector>

struct FComponent;

class CActor
{
public:
    using Super = CActor;

    virtual ~CActor() = default;

    virtual void BeginPlay() {}
    virtual void EndPlay() {}

    virtual void Tick(float DeltaTime);

    void AddComponent(FComponent* Component);
    void RemoveComponent(FComponent* Component);

private:
    std::vector<FComponent*> M_Components;
};
