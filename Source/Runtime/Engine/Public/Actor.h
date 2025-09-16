#pragma once

#include "Object.h"
#include "TransformComponent.h"

class RStaticMeshComponent;

class RActor : public RObject
{
    using Super = RObject;

public:
    RActor();

    static RClass* StaticClass();

    virtual const RClass* GetClass() const override;
    virtual void PostConstruct() override;

    virtual void BeginPlay() {}
    virtual void EndPlay() {}
    virtual void Tick(float DeltaTime) {}

protected:
    RStaticMeshComponent* StaticMeshComponent;

    RTransformComponent* TransformComponent;
};