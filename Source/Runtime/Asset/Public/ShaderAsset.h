#pragma once

#include "AssetManager.h"

struct ShaderAsset
{
    uint32_t* Data;
    size_t WordCount;
};

struct ShaderAssetOptions
{
    std::string EntryPoint;
    std::string TargetProfile;
};

template<> struct AssetTag<ShaderAsset> { static constexpr uint8_t Value = 1; };
template<> struct AssetDefaultOptions<ShaderAsset> { using Type = ShaderAssetOptions; };

template<>
struct AssetLoader<ShaderAsset, ShaderAssetOptions>
{
    static ShaderAsset Load(const AssetLoaderRequest& req, const ShaderAssetOptions& opts);
};