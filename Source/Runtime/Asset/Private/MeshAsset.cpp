#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "MeshAsset.h"

#include <Logger.h>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <variant>

#include <tiny_gltf.h>

#include "Assert.h"
#include "glm/fwd.hpp"

FMeshAsset TAssetLoader<FMeshAsset, FMeshAssetOptions>::Load(const FAssetLoaderRequest &p_Request, const FMeshAssetOptions &p_Options)
{
    //fastgltf::Parser parser;
    //auto Data = fastgltf::GltfDataBuffer::FromPath(p_Request.Path);
    //std::filesystem::path Parent = p_Request.Path;
    //auto Asset = parser.loadGltfBinary(Data.get(), Parent.parent_path(), fastgltf::Options::None);
    //
    FMeshAsset Result;
    //
    //for (const auto& Node : Asset->nodes)
    //{
    //    const auto& Mesh = Asset->meshes[Node.meshIndex.value()];
    //
    //    FMeshAsset::Primitive Submesh;
    //
    //    auto Transform = std::get<fastgltf::TRS>(Node.transform);
    //
    //    glm::vec3 Translation = glm::vec3(Transform.translation.x(), Transform.translation.y(), Transform.translation.z());
    //    glm::vec3 Rotation = glm::vec3(Transform.rotation.x(), Transform.rotation.y(), Transform.rotation.z());
    //    glm::vec3 Scale = glm::vec3(Transform.scale.x(), Transform.scale.y(), Transform.scale.z());
    //
    //    for (const auto& Primitive : Mesh.primitives)
    //    {
    //        const auto* PositionIterator = Primitive.findAttribute("POSITION");
    //        const auto& PositionAccessor = Asset->accessors[PositionIterator->accessorIndex];
    //        const auto& PositionBufferView = Asset->bufferViews[PositionAccessor.bufferViewIndex.value()];
    //        const auto& PositionBuffer = Asset->buffers[PositionBufferView.bufferIndex];
    //
    //        const auto* NormalIterator = Primitive.findAttribute("NORMAL");
    //        const auto& NormalAccessor = Asset->accessors[NormalIterator->accessorIndex];
    //        const auto& NormalBufferView = Asset->bufferViews[NormalAccessor.bufferViewIndex.value()];
    //        const auto& NormalBuffer = Asset->buffers[NormalBufferView.bufferIndex];
    //
    //        const auto* TexCoordIterator = Primitive.findAttribute("TEXCOORD_0");
    //        const auto& TexCoordAccessor = Asset->accessors[TexCoordIterator->accessorIndex];
    //        const auto& TexCoordBufferView = Asset->bufferViews[TexCoordAccessor.bufferViewIndex.value()];
    //        const auto& TexCoordBuffer = Asset->buffers[TexCoordBufferView.bufferIndex];
    //
    //        const auto& PositionArray = std::get<fastgltf::sources::Array>(PositionBuffer.data);
    //        const std::byte* PositionBase = PositionArray.bytes.data();
    //        const std::size_t PositionOffset = PositionBufferView.byteOffset + PositionAccessor.byteOffset;
    //        const glm::vec3* PositionData = reinterpret_cast<const glm::vec3*>(PositionBase + PositionOffset);
    //
    //        const auto& NormalArray = std::get<fastgltf::sources::Array>(NormalBuffer.data);
    //        const std::byte* NormalBase = NormalArray.bytes.data();
    //        const std::size_t NormalOffset = NormalBufferView.byteOffset + NormalAccessor.byteOffset;
    //        const glm::vec3* NormalData = reinterpret_cast<const glm::vec3*>(NormalBase + NormalOffset);
    //
    //        const auto& TexCoordArray = std::get<fastgltf::sources::Array>(TexCoordBuffer.data);
    //        const std::byte* TexCoordBase = TexCoordArray.bytes.data();
    //        const std::size_t TexCoordOffset = TexCoordBufferView.byteOffset + TexCoordAccessor.byteOffset;
    //        const glm::vec2* TexCoordData = reinterpret_cast<const glm::vec2*>(TexCoordBase + TexCoordOffset);
    //
    //        for (size_t Index = 0; Index < PositionAccessor.count; ++Index)
    //        {
    //            FMeshAsset::FVertex Vertex;
    //
    //            const float* Position = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(PositionData) + Index * sizeof(glm::vec3));
    //            Vertex.m_Position = glm::vec3(Position[0], Position[1], Position[2]);
    //
    //            const float* Normal = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(NormalData) + Index * sizeof(glm::vec3));
    //            Vertex.m_Normal = glm::vec3(Normal[0], Normal[1], Normal[2]);
    //
    //            const float* TexCoord = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(TexCoordData) + Index * sizeof(glm::vec2));
    //            Vertex.m_TexCoord = glm::vec2(TexCoord[0], TexCoord[1]);
    //
    //            Submesh.Vertices.push_back(Vertex);
    //            Submesh.Indices.push_back(Index);
    //        }
    //
    //        Result.Node.Primitives.push_back(Submesh);
    //    }
    //}

    tinygltf::Model Model;
    tinygltf::TinyGLTF Loader;

    std::string Error;
    std::string Warning;

    bool bSuccess = Loader.LoadBinaryFromFile(&Model, &Error, &Warning, p_Request.Path);
    RK_ASSERT(bSuccess, "Failed to load mesh asset. Did you use a correct file path?");

    for (const auto& Node : Model.nodes)
    {
        if (Node.mesh == -1)
        {
            continue;
        }

        auto& Mesh = Model.meshes[Node.mesh];

        for (const auto& Primitive : Mesh.primitives)
        {
            FMeshAsset::Primitive Submesh;

            if (Node.translation.size())
            {
                glm::vec3 Translation = glm::vec3(Node.translation[0], Node.translation[1], Node.translation[2]);

                Submesh.Translation = Translation;
            }

            if (Node.rotation.size())
            {
                glm::quat Rotation = glm::quat(Node.rotation[3], Node.rotation[0], Node.rotation[1], Node.rotation[2]);

                Submesh.Rotation = glm::degrees(glm::eulerAngles(Rotation));
            }

            if (Node.scale.size())
            {
                glm::vec3 Scale = glm::vec3(Node.scale[0], Node.scale[1], Node.scale[2]);

                Submesh.Scale = Scale;
            }

            size_t VertexOffset = Submesh.Vertices.size();

            const float* Positions = nullptr;
            const float* TexCoords = nullptr;
            const float* Normals = nullptr;
            const uint8_t* Index = nullptr;

            size_t TexCoordStride = 0;
            size_t PositionStride = 0;
            size_t NormalStride = 0;
            size_t IndexStride = 0;

            size_t VertexCount = 0;
            size_t IndexCount = 0;
            size_t TriangleCount = 0;

            if (Primitive.attributes.find("POSITION") != Primitive.attributes.end())
            {
                const tinygltf::Accessor& Accessor = Model.accessors[Primitive.attributes.at("POSITION")];
                const tinygltf::BufferView& BufferView = Model.bufferViews[Accessor.bufferView];
                const tinygltf::Buffer& Buffer = Model.buffers[BufferView.buffer];

                Positions = reinterpret_cast<const float*>(&Buffer.data[BufferView.byteOffset + Accessor.byteOffset]);
                PositionStride = Accessor.ByteStride(BufferView) ? Accessor.ByteStride(BufferView) : sizeof(glm::vec3);
                VertexCount = Accessor.count;
            }

            if (Primitive.attributes.find("TEXCOORD_0") != Primitive.attributes.end())
            {
                const tinygltf::Accessor& Accessor = Model.accessors[Primitive.attributes.at("TEXCOORD_0")];
                const tinygltf::BufferView& BufferView = Model.bufferViews[Accessor.bufferView];
                const tinygltf::Buffer& Buffer = Model.buffers[BufferView.buffer];

                TexCoords = reinterpret_cast<const float*>(&Buffer.data[BufferView.byteOffset + Accessor.byteOffset]);
                TexCoordStride = Accessor.ByteStride(BufferView) ? Accessor.ByteStride(BufferView) : sizeof(glm::vec2);
            }

            if (Primitive.attributes.find("NORMAL") != Primitive.attributes.end())
            {
                const tinygltf::Accessor& Accessor = Model.accessors[Primitive.attributes.at("NORMAL")];
                const tinygltf::BufferView& BufferView = Model.bufferViews[Accessor.bufferView];
                const tinygltf::Buffer& Buffer = Model.buffers[BufferView.buffer];

                Normals = reinterpret_cast<const float*>(&Buffer.data[BufferView.byteOffset + Accessor.byteOffset]);
                NormalStride = Accessor.ByteStride(BufferView) ? Accessor.ByteStride(BufferView) : sizeof(glm::vec3);
            }

            for (size_t Index = 0; Index < VertexCount; ++Index)
            {
                FVertex Vertex;

                if (Positions)
                {
                    const float* Position = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(Positions) + Index * PositionStride);
                    Vertex.M_Position = glm::vec3(Position[0], Position[1], Position[2]);
                }

                if (TexCoords)
                {
                    const float* TexCoord = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(TexCoords) + Index * TexCoordStride);
                    Vertex.M_TexCoord = glm::vec2(TexCoord[0], TexCoord[1]);
                }

                if (Normals)
                {
                    const float* Normal = reinterpret_cast<const float*>(reinterpret_cast<const uint8_t*>(Normals) + Index * NormalStride);
                    Vertex.M_Normal = glm::vec3(Normal[0], Normal[1], Normal[2]);
                }

                Submesh.Vertices.push_back(Vertex);
            }

            if (Primitive.indices >= 0)
            {
                const tinygltf::Accessor& indexAccessor = Model.accessors[Primitive.indices];
                const tinygltf::BufferView& indexBufferView = Model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& indexBuffer = Model.buffers[indexBufferView.buffer];

                Index = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];
                IndexStride = indexAccessor.componentType;
                IndexCount = indexAccessor.count;
                TriangleCount = indexAccessor.count / 3;

                for (size_t i = 0; i < IndexCount; ++i)
                {
                    uint32_t IndexValue;

                    switch (IndexStride)
                    {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                        {
                            const uint8_t* Value8 = reinterpret_cast<const uint8_t*>(Index);
                            IndexValue = static_cast<uint32_t>(Value8[i]) + static_cast<uint32_t>(VertexOffset);
                            break;
                        }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                        {
                            const uint16_t* Value16 = reinterpret_cast<const uint16_t*>(Index);
                            IndexValue = static_cast<uint32_t>(Value16[i]) + static_cast<uint32_t>(VertexOffset);
                            break;
                        }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                        {
                            const uint32_t* Value32 = reinterpret_cast<const uint32_t*>(Index);
                            IndexValue = static_cast<uint32_t>(Value32[i]) + static_cast<uint32_t>(VertexOffset);
                            break;
                        }
                    }

                    Submesh.Indices.push_back(IndexValue);
                }
            }

            Result.Node.Primitives.push_back(Submesh);
        }
    }
    return Result;
}