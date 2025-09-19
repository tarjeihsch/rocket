#include "World.h"

#include "../../Engine/Public/Engine.h"
#include "Actor.h"
#include "AssetManager.h"
#include "JsonAsset.h"

#include <dlfcn.h>

RActor *CWorld::SpawnActor(RClass *Class)
{
    RActor* Actor = static_cast<RActor*>(ConstructObject(Class));

    Actors.push_back(Actor);

    Actor->BeginPlay();

    return Actor;
}

void CWorld::DestroyActor(RActor *P_Actor)
{
    P_Actor->EndPlay();

    for (size_t i = 0; i < Actors.size(); ++i)
    {
        if (Actors[i] == P_Actor)
        {
            std::swap(Actors[i], Actors.back());

            Actors.pop_back();
            
            break;
        }
    }
}

void CWorld::Tick(float DeltaTime)
{
    for (RActor* Actor : Actors)
    {
        Actor->Tick(DeltaTime);
    }
}

void CWorld::Initialize()
{
    FAssetLoaderRequest Request = { "/home/user/Workspace/Spaceship/Content/World.json" };
    FJsonAssetOptions Options;
    FAssetHandle<FJsonAsset> AssetHandle = FAssetManager::Get().Load<FJsonAsset>(Request, Options);
    FJsonAsset* JsonAsset = FAssetManager::Get().GetAsset(AssetHandle);

    // Top-level is an array
    auto& rootArray = std::get<FJsonAsset::Array>(JsonAsset->Value);

    for (auto& entry : rootArray)
    {
        auto& obj = std::get<FJsonAsset::Object>(entry.Value);
        auto& actors = std::get<FJsonAsset::Array>(obj["Actors"].Value);

        for (auto& actorVal : actors)
        {
            auto& actorObj = std::get<FJsonAsset::Object>(actorVal.Value);

            std::string className = std::get<std::string>(actorObj["Class"].Value);
            std::string symbolName = "Z_Class_" + className + "_StaticClass";

            using GetStaticClassFun = RClass* (*)();
            GetStaticClassFun GetStaticClass = reinterpret_cast<GetStaticClassFun>(dlsym(GetEngine()->GetProjectLibrary(), symbolName.c_str()));

            if (GetStaticClass)
            {
                RClass* Class = GetStaticClass();
                RActor* Actor = SpawnActor(Class);
                Actor->Deserialize(actorObj);
            }
            else
            {
                RClass* Class = RActor::StaticClass();
                RActor* Actor = SpawnActor(Class);
                Actor->Deserialize(actorObj);
            }
        }
    }

    //RClass* Class = GetStaticClass();
    //RActor* Actor = GetWorld()->SpawnActor(Class);
}
void CWorld::Deinitialize()
{
    for (auto Actor : Actors)
    {
        delete Actor;
    }
}
