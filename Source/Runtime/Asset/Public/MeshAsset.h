#pragma once

#include "AssetManager.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

struct MeshAsset
{
    struct Vertex
    {
        alignas(16) glm::vec3 m_Position;
        alignas(16) glm::vec3 m_Normal;
        alignas(16) glm::vec2 m_TexCoord;
    };

    struct Primitive
    {
        std::vector<Vertex> Vertices;
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

struct MeshAssetOptions
{
};

template<> struct AssetTag<MeshAsset> { static constexpr uint8_t Value = 1; };
template<> struct AssetDefaultOptions<MeshAssetOptions> { using Type = MeshAssetOptions; };

template<>
struct AssetLoader<MeshAsset, MeshAssetOptions>
{
    static MeshAsset Load(const AssetLoaderRequest& p_Request, const MeshAssetOptions& p_Options);
};
