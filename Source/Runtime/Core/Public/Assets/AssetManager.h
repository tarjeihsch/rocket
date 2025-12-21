#pragma once

#include "Assets/AssetLoader.h"
#include "Logger.h"
#include "Utils/Timer.h"

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

using FAssetID = uint32_t;

constexpr FAssetID InvalidAssetID = 0xFFFFFFFFu;

inline constexpr FAssetID MakeAssetID(uint8_t type, uint8_t generation, uint16_t index)
{
    return (FAssetID(type) << 24) | (FAssetID(generation) << 16) | index;
}

inline constexpr uint8_t GetType(FAssetID assetId) 
{ 
    return uint8_t(assetId >> 24);
}

inline constexpr uint8_t GetGeneration(FAssetID assetId) 
{ 
    return uint8_t(assetId >> 16);
}

inline constexpr uint16_t GetIndex(FAssetID assetId) 
{ 
    return uint16_t(assetId);
}

template<typename T>
struct FAssetHandle
{
    FAssetID ID = InvalidAssetID;
    bool IsValid() const { return ID != InvalidAssetID; }
};

struct FAssetLoaderRequest
{
    std::string Path;
};

template<typename T>
class FAssetPool
{
public:
    FAssetHandle<T> Alloc()
    {
        uint16_t Index;
        if (FreeList.empty())
        {
            Index = static_cast<uint16_t>(Items.size());
            Items.emplace_back();
            Generations.push_back(1);
        }
        else
        {
            Index = FreeList.back();
            FreeList.pop_back();
        }

        return { MakeAssetID(AssetTag<T>::Value, Generations[Index], Index) };
    }

    void Free(FAssetHandle<T> Handle)
    {
        if (!Handle.IsValid())
        {
            return;
        }

        const uint16_t index = GetIndex(Handle.ID);
        const uint8_t generation = GetGeneration(Handle.ID);
        if (index >= Items.size() || Generations[index] != generation)
        {
            // Stale
            return;
        }

        ++Generations[index];
        FreeList.push_back(index);
        Items[index] = T{};
    }

    T* Get(FAssetHandle<T> Handle)
    {
        const uint16_t Index = GetIndex(Handle.ID);
        const uint8_t Generation = GetGeneration(Handle.ID);
        return (Index < Items.size() && Generations[Index] == Generation) ? &Items[Index] : nullptr;
    }

    const T* Get(FAssetHandle<T> Handle) const
    {
        const uint16_t Index = GetIndex(Handle.ID);
        const uint8_t Generation = GetGeneration(Handle.ID);
        return (Index < Items.size() && Generations[Index] == Generation) ? &Items[Index] : nullptr;
    }

private:
    std::deque<T> Items;
    std::vector<uint8_t> Generations;
    std::vector<uint16_t> FreeList;
};

class FAssetManager
{
public:
    template<typename T, typename TOptions>
    FAssetHandle<T> Load(const FAssetLoaderRequest& Request, const TOptions& Options)
    {
        auto Handle = GetPool<T>().Alloc();
        T* Slot = GetPool<T>().Get(Handle);
        *Slot = AssetLoader<T, TOptions>::Load(Request, Options);
        return Handle;
    }

    template<typename T>
    void Unload(FAssetHandle<T> Handle)
    {
        GetPool<T>().Free(Handle);
    }

    template<typename T>
    T* GetAsset(FAssetHandle<T> Handle)
    {
        return GetPool<T>().Get(Handle);
    }

    static FAssetManager& Get()
    {
        static FAssetManager Instance;
        return Instance;
    }

private:
    template<typename T>
    FAssetPool<T>& GetPool()
    {
        static FAssetPool<T> Pool;
        return Pool;
    }
};
