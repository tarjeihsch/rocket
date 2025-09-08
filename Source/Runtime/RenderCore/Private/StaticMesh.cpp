#include "StaticMesh.h"

#include "AssetManager.h"
#include "Common/RHIBuffer.h"
#include "Common/RHI.h"
#include "MeshAsset.h"

CStaticMesh::CStaticMesh(const char* InFilePath)
    : M_VertexBuffer(nullptr),
    M_IndexBuffer(nullptr),
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

    FRHIBufferDesc VertexBufferDesc = FRHIBufferDesc::CreateVertexBuffer(MeshAsset->Node.Primitives[0].Vertices.size() * sizeof(FRHIVertex));
    FRHIBufferDesc IndexBufferDesc = FRHIBufferDesc::CreateIndexBuffer(MeshAsset->Node.Primitives[0].Indices.size() * sizeof(uint32_t));

    M_VertexBuffer = GetRHI()->RHICreateBuffer(VertexBufferDesc);
    M_IndexBuffer = GetRHI()->RHICreateBuffer(IndexBufferDesc);

    M_VertexCount = MeshAsset->Node.Primitives[0].Vertices.size();
    M_IndexCount = MeshAsset->Node.Primitives[0].Indices.size();

    GetRHI()->RHIUploadDataBlocking(M_VertexBuffer, MeshAsset->Node.Primitives[0].Vertices.data(), MeshAsset->Node.Primitives[0].Vertices.size() * sizeof(FRHIVertex));
    GetRHI()->RHIUploadDataBlocking(M_IndexBuffer, MeshAsset->Node.Primitives[0].Indices.data(), MeshAsset->Node.Primitives[0].Indices.size() * sizeof(uint32_t));
}

void CStaticMesh::Shutdown()
{
    delete M_VertexBuffer;
    delete M_IndexBuffer;
}