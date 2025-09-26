#include "Actor.h"

#include "../../Engine/Public/EngineBase.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "TransformComponent.h"
#include "World.h"

RClass *RActor::StaticClass()
{
    static RActor Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RActor), alignof(RActor), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

const RClass *RActor::GetClass() const
{
    return StaticClass();
}

void RActor::PostConstruct()
{
    StaticMeshComponent = ConstructObject<RStaticMeshComponent>();

    GEngine->GetWorld()->GetTickTaskManager()->AddTickFunction<&RActor::Tick>(ETickGroup::Default, this);
}

void RActor::Deserialize(const FJsonAsset::Object &Obj)
{
    if (auto it = Obj.find("StaticMesh"); it != Obj.end())
    {
        RStaticMesh* StaticMesh = new RStaticMesh(std::string(std::get<std::string>(it->second.Value)).c_str());
        StaticMeshComponent->SetStaticMesh(StaticMesh);
    }

    if (auto it = Obj.find("Location"); it != Obj.end()) {
        const auto& loc = std::get<FJsonAsset::Object>(it->second.Value);
        float X = (float)std::get<double>(loc.at("x").Value);
        float Y = (float)std::get<double>(loc.at("y").Value);
        float Z = (float)std::get<double>(loc.at("z").Value);

        StaticMeshComponent->SetLocation(glm::vec3{X, Y, Z});
    }
}

void RActor::AddComponent(RComponent* Component)
{
    Components.push_back(Component);
}

RComponent *RActor::GetComponentOfClass(RClass *Class) const
{
    for (RComponent *Component : Components)
    {
        if (Component->GetClass()->IsA(Class))
        {
            return Component;
        }
    }
}

RActor &RActor::operator<<(const FJsonAsset &json)
{
    if (!std::holds_alternative<FJsonAsset::Object>(json.Value)) throw std::runtime_error("Actor JSON is not an object");

    Deserialize(std::get<FJsonAsset::Object>(json.Value));

    return *this;
}