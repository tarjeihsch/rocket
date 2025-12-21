#pragma once

#include <string>

struct AssetLoaderRequest
{
    std::string filePath;
};

template<typename T> 
struct AssetTag;

template<typename T, typename TOptions> 
struct AssetLoader;