#pragma once

#include "AssetManager.h"

struct ShaderAsset
{
    std::vector<uint32_t> Code;

    const uint32_t* Data() const { return Code.data(); }
    size_t WordCount() const { return Code.size(); }
};

struct ShaderAssetOptions
{
    std::string EntryPoint;
    std::string TargetProfile;
};

template<> struct TAssetTag<ShaderAsset> { static constexpr uint8_t Value = 1; };
template<> struct TAssetDefaultOptions<ShaderAsset> { using Type = ShaderAssetOptions; };

template<>
struct TAssetLoader<ShaderAsset, ShaderAssetOptions>
{
    static ShaderAsset Load(const FAssetLoaderRequest& req, const ShaderAssetOptions& opts);
};