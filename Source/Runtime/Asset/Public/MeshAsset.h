#pragma once

#include "AssetManager.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct FMeshAsset
{
    struct FVertex
    {
        alignas(16) glm::vec3 m_Position;
        alignas(16) glm::vec3 m_Normal;
        alignas(16) glm::vec2 m_TexCoord;
    };

    struct Primitive
    {
        std::vector<FVertex> Vertices;
        std::vector<uint32_t> Indices;

        glm::vec3 Translation;
        glm::vec3 Rotation;
        glm::vec3 Scale;
    };

    struct Node
    {
        std::vector<Primitive> Primitives;
    };

    Node Node = {};
};

struct FMeshAssetOptions
{
};

template<> struct TAssetTag<FMeshAsset> { static constexpr uint8_t Value = 1; };
template<> struct TAssetDefaultOptions<FMeshAssetOptions> { using Type = FMeshAssetOptions; };

template<>
struct TAssetLoader<FMeshAsset, FMeshAssetOptions>
{
    static FMeshAsset Load(const FAssetLoaderRequest& p_Request, const FMeshAssetOptions& p_Options);
};
