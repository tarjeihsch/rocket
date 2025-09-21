#pragma once

#include "JsonAsset.h"
#include "Object.h"
#include "Tickable.h"

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

    inline RStaticMeshComponent* GetStaticMeshComponent() const;

    RActor& operator<<(const FJsonAsset& json);

protected:
    TObjectPtr<RStaticMeshComponent> StaticMeshComponent;
};

inline RStaticMeshComponent *RActor::GetStaticMeshComponent() const
{
    return StaticMeshComponent;
}