#pragma once

#include "Component.h"
#include "Delegate.h"
#include "JsonAsset.h"
#include "Object.h"
#include "Tickable.h"
#include <span>

class RComponent;
class RSceneComponent;
class RStaticMeshComponent;

class RActor : public RObject
{
    using Super = RObject;

public:
    static RClass* StaticClass();

    virtual const RClass* GetClass() const override;
    virtual void PostConstruct(RWorld* World, RObject* Owner) override;

    virtual void BeginPlay() {}
    virtual void EndPlay() {}

    template<typename TComponent>
    TComponent* AddComponent();
    RComponent* AddComponent(RClass* Class);

    std::span<RComponent*> GetComponents();

private:
    std::vector<RComponent*> Components;
};

template <typename TComponent>
TComponent* RActor::AddComponent()
{
    static_assert(std::is_base_of<RComponent, TComponent>::value, "TComponent must derive from RComponent.");
    RComponent* BaseComponent = AddComponent(TComponent::StaticClass());
    TComponent* NewComponent = static_cast<TComponent*>(BaseComponent);
    return NewComponent;
}