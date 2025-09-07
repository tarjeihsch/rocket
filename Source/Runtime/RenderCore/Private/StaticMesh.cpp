#include "StaticMesh.h"

#include "AssetManager.h"
#include "Common/RHIBuffer.h"
#include "Common/RHI.h"
#include "MeshAsset.h"

FStaticMesh::FStaticMesh(const char* InFilePath)
    : VertexBuffer(nullptr),
    IndexBuffer(nullptr),
    FilePath(InFilePath)
{
    Init();
}

FStaticMesh::~FStaticMesh()
{
    Shutdown();
}

void FStaticMesh::Init()
{
    FAssetLoaderRequest Request = { FilePath };
    FMeshAssetOptions Options;

    FAssetHandle<FMeshAsset> AssetHandle = FAssetManager::Get().Load<FMeshAsset>(Request, Options);
    FMeshAsset* MeshAsset = FAssetManager::Get().GetAsset(AssetHandle);

    FRHIBufferDesc VertexBufferDesc = FRHIBufferDesc::CreateVertexBuffer(MeshAsset->Node.Primitives[0].Vertices.size() * sizeof(FRHIVertex));
    FRHIBufferDesc IndexBufferDesc = FRHIBufferDesc::CreateIndexBuffer(MeshAsset->Node.Primitives[0].Indices.size() * sizeof(uint32_t));

    VertexBuffer = GetRHI()->RHICreateBuffer(VertexBufferDesc);
    IndexBuffer = GetRHI()->RHICreateBuffer(IndexBufferDesc);

    VertexCount = MeshAsset->Node.Primitives[0].Vertices.size();
    IndexCount = MeshAsset->Node.Primitives[0].Indices.size();

    GetRHI()->RHIUploadDataBlocking(VertexBuffer, MeshAsset->Node.Primitives[0].Vertices.data(), MeshAsset->Node.Primitives[0].Vertices.size() * sizeof(FRHIVertex));
    GetRHI()->RHIUploadDataBlocking(IndexBuffer, MeshAsset->Node.Primitives[0].Indices.data(), MeshAsset->Node.Primitives[0].Indices.size() * sizeof(uint32_t));
}

void FStaticMesh::Shutdown()
{
    delete VertexBuffer;
    delete IndexBuffer;
}