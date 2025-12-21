#pragma once

#include "Assets/AssetLoader.h"

#include <vector>
#include <cstdint>

struct IRByteCode
{
    std::vector<uint8_t> Code;

    const uint8_t* Data() const { return Code.data(); }
    size_t WordCount() const { return Code.size(); }
};

struct IRByteCodeLoaderOptions
{
    std::string EntryPoint;
    std::string TargetProfile;
};

template<> 
struct AssetTag<IRByteCode>
{ 
    static constexpr uint8_t Value = 1; 
};

template<>
struct AssetLoader<IRByteCode, IRByteCodeLoaderOptions>
{
    static IRByteCode Load(const AssetLoaderRequest& loaderRequest, const IRByteCodeLoaderOptions& loaderOptions);
};