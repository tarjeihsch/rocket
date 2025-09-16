#include "Actor.h"

#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "TransformComponent.h"

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
}

void RActor::Deserialize(const FJsonAsset::Object &Obj)
{
    if (auto it = Obj.find("StaticMesh"); it != Obj.end())
    {
        CStaticMesh* StaticMesh = new CStaticMesh(std::string(std::get<std::string>(it->second.Value)).c_str());
        StaticMeshComponent->SetStaticMesh(StaticMesh);
    }

    if (auto it = Obj.find("Location"); it != Obj.end()) {
        const auto& loc = std::get<FJsonAsset::Object>(it->second.Value);
        float X = (float)std::get<double>(loc.at("x").Value);
        float Y = (float)std::get<double>(loc.at("y").Value);
        float Z = (float)std::get<double>(loc.at("z").Value);

        StaticMeshComponent->Transform.M_Translation = (glm::vec3{X, Y, Z});
    }
}

RActor &RActor::operator<<(const FJsonAsset &json)
{
    if (!std::holds_alternative<FJsonAsset::Object>(json.Value)) throw std::runtime_error("Actor JSON is not an object");

    Deserialize(std::get<FJsonAsset::Object>(json.Value));

    return *this;
}