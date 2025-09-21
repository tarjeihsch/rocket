#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include "Logger.h"
#include "Timer.h"

using FAssetID = uint32_t;
constexpr FAssetID InvalidAssetID = 0xFFFFFFFFu;

inline constexpr FAssetID MakeAssetID(uint8_t pType, uint8_t pGeneration, uint16_t pIndex)
{
    return (FAssetID(pType) << 24) | (FAssetID(pGeneration) << 16) | pIndex;
}

inline constexpr uint8_t GetType(FAssetID pAssetID) { return uint8_t(pAssetID >> 24); }
inline constexpr uint8_t GetGeneration(FAssetID pAssetID) { return uint8_t(pAssetID >> 16); }
inline constexpr uint16_t GetIndex(FAssetID pAssetID) { return uint16_t(pAssetID); }

template<typename T>
struct FAssetHandle
{
    // TODO: Use DTOR to Free this asset

    FAssetID ID = InvalidAssetID;
    bool IsValid() const { return ID != InvalidAssetID; }
};

struct FAssetLoaderRequest
{
    std::string Path;
};

struct NoOptions {};

template<typename T> struct TAssetTag; // Specialized per asset type
template<typename T> struct TAssetDefaultOptions { using Type = NoOptions; }; // Default = no options
template<typename T, typename Options = typename TAssetDefaultOptions<T>::Type> struct TAssetLoader; // Specializations must implement function: static T Load(Request, Options);

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

        return { MakeAssetID(TAssetTag<T>::Value, Generations[Index], Index) };
    }

    void Free(FAssetHandle<T> Handle)
    {
        if (!Handle.IsValid())
        {
            return;
        }

        const uint16_t Index = GetIndex(Handle.ID);
        const uint8_t Generation = GetGeneration(Handle.ID);
        if (Index >= Items.size() || Generations[Index] != Generation)
        {
            // Stale
            return;
        }

        ++Generations[Index];
        FreeList.push_back(Index);
        Items[Index] = T{};
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
    template<typename T, typename TOptions = NoOptions>
    FAssetHandle<T> Load(const FAssetLoaderRequest& Request, const TOptions& Options = TOptions{})
    {
        STimer Timer;
        auto Handle = GetPool<T>().Alloc();
        T* Slot = GetPool<T>().Get(Handle);
        *Slot = TAssetLoader<T, TOptions>::Load(Request, Options);
        RK_LOG_DEBUG("TAssetLoader::Load():\n\t\tPath: {}\n\t\tTime: {}s", Request.Path, Timer.GetElapsedTimeAsSeconds());
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
