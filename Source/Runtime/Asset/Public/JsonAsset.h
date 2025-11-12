#pragma once

#include <AssetManager.h>
#include <unordered_map>
#include <variant>

struct FJsonAsset
{
    using Object = std::unordered_map<std::string, FJsonAsset>;
    using Array  = std::vector<FJsonAsset>;

    std::variant<std::nullptr_t, bool, double, std::string, Array, Object> Value;
};

struct FJsonAssetOptions
{
};

template<> struct TAssetTag<FJsonAsset> { static constexpr uint8_t Value = 1; };
template<> struct TAssetDefaultOptions<FJsonAssetOptions> { using Type = FJsonAssetOptions; };

template<>
struct TAssetLoader<FJsonAsset, FJsonAssetOptions>
{
    static FJsonAsset Load(const FAssetLoaderRequest& P_Request, const FJsonAssetOptions& P_Options);
};
