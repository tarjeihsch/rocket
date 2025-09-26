#pragma once

#include "Component.h"
#include "JsonAsset.h"
#include "Object.h"
#include "Tickable.h"

class RComponent;
class RStaticMeshComponent;

class RActor : public RObject
{
    using Super = RObject;

public:
    static RClass* StaticClass();

    virtual const RClass* GetClass() const override;
    virtual void PostConstruct() override;

    virtual void BeginPlay() {}
    virtual void EndPlay() {}
    virtual void Tick(float DeltaTime) {}

    virtual void Deserialize(const FJsonAsset::Object& Obj);

    void AddComponent(RComponent *Component);
    RComponent* GetComponentOfClass(RClass* Class) const;

    template<typename TComponent>
    TComponent* AddComponent();

    template<typename TComponent>
    TComponent* GetComponentOfClass() const;

    inline RStaticMeshComponent* GetStaticMeshComponent() const;

    RActor& operator<<(const FJsonAsset& json);

protected:
    TObjectPtr<RStaticMeshComponent> StaticMeshComponent;

private:
    std::vector<RComponent*> Components;
};

template <typename TComponent>
TComponent* RActor::AddComponent()
{
    TComponent* Component = ConstructObject<TComponent>();
    AddComponent(Component);
    return Component;
}

template <typename TComponent>
TComponent* RActor::GetComponentOfClass() const
{
    return GetComponentOfClass(TComponent::StaticClass());
}

inline RStaticMeshComponent *RActor::GetStaticMeshComponent() const
{
    return StaticMeshComponent;
}
