#include "World.h"

#include "Actor.h"
#include "AssetManager.h"
#include "JsonAsset.h"
#include "Module.h"
#include "../../Renderer/Public/PrimitiveManager.h"

#ifdef RK_PLATFORM_LINUX
#include <dlfcn.h>
#endif

RActor *RWorld::SpawnActor(RClass *Class)
{
    RActor* Actor = ConstructObject<RActor>(this, nullptr);

    Actors.push_back(Actor);

    Actor->BeginPlay();

    return Actor;
}

RWorld::RWorld()
{
	TickTaskManager = new RTickTaskManager();

	PrimitiveProxyManager = new RRenderProxyManager();
}

RWorld::~RWorld()
{
    delete TickTaskManager;

    delete PrimitiveProxyManager;
}

void RWorld::DestroyActor(RActor *P_Actor)
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

void RWorld::Tick(float DeltaTime)
{
    TickTaskManager->TickFrame(DeltaTime);
}

void RWorld::Initialize()
{
    //FAssetLoaderRequest Request = { "/home/user/Workspace/Spaceship/Content/World.json" };
    //FJsonAssetOptions Options;
    //FAssetHandle<FJsonAsset> AssetHandle = FAssetManager::Get().Load<FJsonAsset>(Request, Options);
    //FJsonAsset* JsonAsset = FAssetManager::Get().GetAsset(AssetHandle);
//
    //// Top-level is an array
    //auto& rootArray = std::get<FJsonAsset::Array>(JsonAsset->Value);
//
    //for (auto& entry : rootArray)
    //{
    //    auto& obj = std::get<FJsonAsset::Object>(entry.Value);
    //    auto& actors = std::get<FJsonAsset::Array>(obj["Actors"].Value);
//
    //    for (auto& actorVal : actors)
    //    {
    //        auto& actorObj = std::get<FJsonAsset::Object>(actorVal.Value);
//
    //        std::string className = std::get<std::string>(actorObj["Class"].Value);
    //        std::string symbolName = "Z_Class_" + className + "_StaticClass";
//
    //        using GetStaticClassFun = RClass* (*)();
    //        GetStaticClassFun GetStaticClass = reinterpret_cast<GetStaticClassFun>(GScriptModule->GetSymbol((GScriptModule, symbolName.c_str())));
//
    //        if (GetStaticClass)
    //        {
    //            RClass* Class = GetStaticClass();
    //            RActor* Actor = SpawnActor(Class);
    //            //Actor->Deserialize(actorObj);
    //        }
    //        else
    //        {
    //            RClass* Class = RActor::StaticClass();
    //            RActor* Actor = SpawnActor(Class);
    //            //Actor->Deserialize(actorObj);
    //        }
    //    }
    //}

    //RClass* Class = GetStaticClass();
    //RActor* Actor = GetWorld()->SpawnActor(Class);
}
void RWorld::Deinitialize()
{
    for (auto Actor : Actors)
    {
        delete Actor;
    }

    delete TickTaskManager;

    delete PrimitiveProxyManager;
}

std::span<RActor*> RWorld::GetActors()
{
    return Actors;
}
