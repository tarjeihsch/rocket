#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

using AssetID = uint32_t;
constexpr AssetID InvalidAssetID = 0xFFFFFFFFu;

inline constexpr AssetID MakeAssetID(uint8_t pType, uint8_t pGeneration, uint16_t pIndex)
{
    return (AssetID(pType) << 24) | (AssetID(pGeneration) << 16) | pIndex;
}

inline constexpr uint8_t GetType(AssetID pAssetID) { return uint8_t(pAssetID >> 24); }
inline constexpr uint8_t GetGeneration(AssetID pAssetID) { return uint8_t(pAssetID >> 16); }
inline constexpr uint16_t GetIndex(AssetID pAssetID) { return uint16_t(pAssetID); }

template<typename T>
struct AssetHandle
{
    AssetID ID = InvalidAssetID;
    bool IsValid() const { return ID != InvalidAssetID; }
};

struct AssetLoaderRequest
{
    std::string Path;
};

struct NoOptions {};

template<typename T> struct AssetTag; // Specialized per asset type
template<typename T> struct AssetDefaultOptions { using Type = NoOptions; }; // Default = no options

template<typename T, typename Options = typename AssetDefaultOptions<T>::Type>
struct AssetLoader; // Specializations must implement function: static T Load(Request, Options);

template<typename T>
struct AssetLoader<T, NoOptions>
{
    static T Load(const AssetLoaderRequest& Request);
};

template<typename T>
using AssetOptionsT = typename AssetDefaultOptions<T>::Type;

template<typename T>
class AssetPool
{
public:
    AssetHandle<T> Alloc()
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

    void Free(AssetHandle<T> Handle)
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

    T* Get(AssetHandle<T> Handle)
    {
        const uint16_t Index = GetIndex(Handle.ID);
        const uint8_t Generation = GetGeneration(Handle.ID);
        return (Index < Items.size() && Generations[Index] == Generation) ? &Items[Index] : nullptr;
    }

    const T* Get(AssetHandle<T> Handle) const
    {
        const uint16_t Index = GetIndex(Handle.ID);
        const uint8_t Generation = GetGeneration(Handle.ID);
        return (Index < Items.size() && Generations[Index] == Generation) ? &Items[Index] : nullptr;
    }

private:
    std::vector<T> Items;
    std::vector<uint8_t> Generations;
    std::vector<uint16_t> FreeList;
};

class AssetManager
{
public:
    template<typename T, typename Options = AssetOptionsT<T>>
    AssetHandle<T> Load(const AssetLoaderRequest& Request, const Options& opts = Options{})
    {
        auto Handle = GetPool<T>().Alloc();
        T* Slot = GetPool<T>().Get(Handle);
        *Slot = AssetLoader<T, Options>::Load(Request, opts);
        return Handle;
    }

    template<typename T>
    void Unload(AssetHandle<T> Handle)
    {
        GetPool<T>().Free(Handle);
    }

    template<typename T>
    T* Get(AssetHandle<T> Handle)
    {
        return GetPool<T>().Get(Handle);
    }

private:
    template<typename T>
    AssetPool<T>& GetPool()
    {
        static AssetPool<T> Pool;
        return Pool;
    }
};
