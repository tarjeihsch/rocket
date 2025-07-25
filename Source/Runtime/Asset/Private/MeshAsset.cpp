#include "MeshAsset.h"

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <variant>
#include <Logger.h>

MeshAsset AssetLoader<MeshAsset, MeshAssetOptions>::Load(const AssetLoaderRequest &p_Request, const MeshAssetOptions &p_Options)
{
    fastgltf::Parser parser;
    auto Data = fastgltf::GltfDataBuffer::FromPath(p_Request.Path);
    std::filesystem::path Parent = p_Request.Path;
    auto Asset = parser.loadGltfBinary(Data.get(), Parent.parent_path(), fastgltf::Options::None);

    MeshAsset Result;

    for (const auto& Node : Asset->nodes)
    {
        const auto& Mesh = Asset->meshes[Node.meshIndex.value()];

        MeshAsset::Primitive Submesh;

        auto Transform = std::get<fastgltf::TRS>(Node.transform);

        glm::vec3 Translation = glm::vec3(Transform.translation.x(), Transform.translation.y(), Transform.translation.z());
        glm::vec3 Rotation = glm::vec3(Transform.rotation.x(), Transform.rotation.y(), Transform.rotation.z());
        glm::vec3 Scale = glm::vec3(Transform.scale.x(), Transform.scale.y(), Transform.scale.z());

        for (const auto& Primitive : Mesh.primitives)
        {
            const auto* PositionIterator = Primitive.findAttribute("POSITION");
            const auto& PositionAccessor = Asset->accessors[PositionIterator->accessorIndex];
            const auto& PositionBufferView = Asset->bufferViews[PositionAccessor.bufferViewIndex.value()];
            const auto& PositionBuffer = Asset->buffers[PositionBufferView.bufferIndex];
            
            const auto* NormalIterator = Primitive.findAttribute("NORMAL");
            const auto& NormalAccessor = Asset->accessors[NormalIterator->accessorIndex];
            const auto& NormalBufferView = Asset->bufferViews[NormalAccessor.bufferViewIndex.value()];
            const auto& NormalBuffer = Asset->buffers[NormalBufferView.bufferIndex];

            const auto* TexCoordIterator = Primitive.findAttribute("TEXCOORD_0");
            const auto& TexCoordAccessor = Asset->accessors[TexCoordIterator->accessorIndex];
            const auto& TexCoordBufferView = Asset->bufferViews[TexCoordAccessor.bufferViewIndex.value()];
            const auto& TexCoordBuffer = Asset->buffers[TexCoordBufferView.bufferIndex];

            const auto& PositionArray = std::get<fastgltf::sources::Array>(PositionBuffer.data);
            const std::byte* PositionBase = PositionArray.bytes.data();
            const std::size_t PositionOffset = PositionBufferView.byteOffset + PositionAccessor.byteOffset;
            const glm::vec3* PositionData = reinterpret_cast<const glm::vec3*>(PositionBase + PositionOffset);

            const auto& NormalArray = std::get<fastgltf::sources::Array>(NormalBuffer.data);
            const std::byte* NormalBase = NormalArray.bytes.data();
            const std::size_t NormalOffset = NormalBufferView.byteOffset + NormalAccessor.byteOffset;
            const glm::vec3* NormalData = reinterpret_cast<const glm::vec3*>(NormalBase + NormalOffset);

            const auto& TexCoordArray = std::get<fastgltf::sources::Array>(TexCoordBuffer.data);
            const std::byte* TexCoordBase = TexCoordArray.bytes.data();
            const std::size_t TexCoordOffset = TexCoordBufferView.byteOffset + TexCoordAccessor.byteOffset;
            const glm::vec2* TexCoordData = reinterpret_cast<const glm::vec2*>(TexCoordBase + TexCoordOffset);

            for (size_t Index = 0; Index < PositionAccessor.count; ++Index)
            {
                MeshAsset::Vertex Vertex;

                const float* Position = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(PositionData) + Index * sizeof(glm::vec3));
                Vertex.m_Position = glm::vec3(Position[0], Position[1], Position[2]);

                const float* Normal = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(NormalData) + Index * sizeof(glm::vec3));
                Vertex.m_Normal = glm::vec3(Normal[0], Normal[1], Normal[2]);

                const float* TexCoord = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(TexCoordData) + Index * sizeof(glm::vec2));
                Vertex.m_TexCoord = glm::vec2(TexCoord[0], TexCoord[1]);

                Submesh.Vertices.push_back(Vertex);
            }

            Result.Node.Primitives.push_back(Submesh);
        }
    }
    return Result;
}