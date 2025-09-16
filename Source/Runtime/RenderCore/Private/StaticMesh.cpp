#include "StaticMesh.h"

#include "AssetManager.h"
#include "Common/RHIBuffer.h"
#include "Common/RHI.h"
#include "MeshAsset.h"

CStaticMesh::CStaticMesh(const char* InFilePath)
    : M_VertexBuffer(nullptr),
    M_IndexBuffer(nullptr),
    M_VertexCount(0),
    M_IndexCount(0),
    FilePath(InFilePath)
{
    Init();
}

CStaticMesh::~CStaticMesh()
{
    Shutdown();
}

void CStaticMesh::Init()
{
    FAssetLoaderRequest Request = { FilePath };
    FMeshAssetOptions Options;

    FAssetHandle<FMeshAsset> AssetHandle = FAssetManager::Get().Load<FMeshAsset>(Request, Options);
    FMeshAsset* MeshAsset = FAssetManager::Get().GetAsset(AssetHandle);

    std::vector<FVertex> Vertices;
    std::vector<uint32_t> Indices;

    for (const auto& Primitive : MeshAsset->Node.Primitives)
    {
        FPrimitiveMeshSection& Section = M_Sections.emplace_back();
        Section.M_IndexCount = Primitive.Indices.size();
        Section.M_VertexCount = Primitive.Vertices.size();
        Section.M_VertexOffset = M_VertexCount;
        Section.M_IndexOffset = M_IndexCount;
        Section.M_LocalMatrix =
            glm::translate(glm::mat4(1.0f), Primitive.Translation) *
            glm::toMat4(glm::quat(glm::radians(Primitive.Rotation))) *
            glm::scale(glm::mat4(1.0f), Primitive.Scale);


        Vertices.insert(Vertices.end(), Primitive.Vertices.begin(), Primitive.Vertices.end());
        Indices.insert(Indices.end(), Primitive.Indices.begin(), Primitive.Indices.end());

        M_VertexCount += Section.M_VertexCount;
        M_IndexCount += Section.M_IndexCount;
    }

    FRHIBufferDesc VertexBufferDesc = FRHIBufferDesc::CreateVertexBuffer(Vertices.size() * sizeof(FVertex));
    FRHIBufferDesc IndexBufferDesc = FRHIBufferDesc::CreateIndexBuffer(Indices.size() * sizeof(uint32_t));

    M_VertexBuffer = GetRHI()->RHICreateBuffer(VertexBufferDesc);
    M_IndexBuffer = GetRHI()->RHICreateBuffer(IndexBufferDesc);

    M_VertexCount = Vertices.size();
    M_IndexCount = Indices.size();

    GetRHI()->RHIUploadDataBlocking(M_VertexBuffer, Vertices.data(), Vertices.size() * sizeof(FVertex));
    GetRHI()->RHIUploadDataBlocking(M_IndexBuffer, Indices.data(), Indices.size() * sizeof(uint32_t));

    FAssetManager::Get().Unload(AssetHandle);
}

void CStaticMesh::Shutdown()
{
    delete M_VertexBuffer;
    delete M_IndexBuffer;
}