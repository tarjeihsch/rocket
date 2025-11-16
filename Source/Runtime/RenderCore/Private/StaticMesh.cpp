#include "StaticMesh.h"

#include "AssetManager.h"
#include "Common/RHIBuffer.h"
#include "Common/RHI.h"
#include "Common/RHIDrawCommand.h"
#include "MeshAsset.h"
#include "Primitive.h"

RStaticMesh::RStaticMesh(const char* InFilePath)
    : M_VertexBuffer(nullptr),
    M_IndexBuffer(nullptr),
    M_VertexCount(0),
    M_IndexCount(0),
    FilePath(InFilePath),
      RPrimitive({}, {})
{
    Init();
}

RStaticMesh::~RStaticMesh()
{
    Shutdown();
}

void RStaticMesh::Draw(FRHICommandList& RHICmdList) const
{
	//RPrimitive::Draw(RHICmdList);
	//
	//for (const auto& Section : Sections)
	//{
	//    FRHIBufferMemoryPacker ObjectData(ERHIBufferMemoryPackerAlignmentLayout::STD430);
	//    ObjectData.Write(Proxy.Transform.ModelMatrix * Section.LocalMatrix);
	//    ObjectData.Write(Proxy.Transform.InverseModelMatrix);
	//    std::shared_ptr<FRHIBuffer> ObjectInterface_GPUBuffer = GetRHI()->RHIBindBuffer(ObjectData.GetData(), ObjectData.GetSize());
	//
	//    FRHIDrawCommand DrawCommand = {};
	//    DrawCommand.VertexBuffer = M_VertexBuffer;
	//    DrawCommand.IndexBuffer = M_IndexBuffer;
	//    DrawCommand.IndexCount = Section.M_IndexCount;
	//    DrawCommand.VertexBufferOffset = Section.M_VertexOffset;
	//    DrawCommand.IndexBufferOffset = Section.M_IndexOffset;
	//    DrawCommand.IndexType = FRHIDrawCommand::EIndexType::U32;
	//
	//    FRHIDrawParams DrawParams = {};
	//    DrawParams.DrawParam_U32_1 = CameraInterface_GPUBuffer->GetBindingIndex(); // Camera ID
	//    DrawParams.DrawParam_U32_2 = ObjectInterface_GPUBuffer->GetBindingIndex(); // Instance ID
	//
	//    GetRHI()->RHICmdDrawIndexed(RHICmdList, DrawCommand, DrawParams);
	//}
}

void RStaticMesh::Init()
{
    FAssetLoaderRequest Request = { FilePath };
    FMeshAssetOptions Options;

    FAssetHandle<FMeshAsset> AssetHandle = FAssetManager::Get().Load<FMeshAsset>(Request, Options);
    FMeshAsset* MeshAsset = FAssetManager::Get().GetAsset(AssetHandle);

    std::vector<FVertex> Vertices;
    std::vector<uint32_t> Indices;

    for (const auto& Primitive : MeshAsset->Node.Primitives)
    {
        FMeshChildSection& Section = Sections.emplace_back();
        Section.M_IndexCount = Primitive.Indices.size();
        Section.M_VertexCount = Primitive.Vertices.size();
        Section.M_VertexOffset = M_VertexCount;
        Section.M_IndexOffset = M_IndexCount;
        Section.LocalMatrix =
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

void RStaticMesh::Shutdown()
{
    delete M_VertexBuffer;
    delete M_IndexBuffer;
}
