#include "Primitive.h"

#include "Material.h"
#include "Common/RHI.h"
#include "Common/RHIDrawCommand.h"
#include "Common/RHIBuffer.h"

RPrimitive::RPrimitive(const std::vector<RVertex>& VertexData, const std::vector<RIndex>& IndexData)
{
    FRHIBufferDesc VertexBufferDesc = {};
    VertexBufferDesc.Size = VertexData.size() * sizeof(RVertex);
    VertexBufferDesc.Lifetime = ERHIResourceLifetime::Transient;
    VertexBufferDesc.Usage = ERHIBufferUsage::Vertex;
    VertexBufferDesc.Memory = ERHIResourceMemory::HostVisible;
    VertexBuffer.reset(GetRHI()->RHICreateBuffer(VertexBufferDesc));
    VertexBuffer->Map(VertexData.data(), VertexBufferDesc.Size, 0);

    FRHIBufferDesc IndexBufferDesc = {};
    IndexBufferDesc.Size = IndexData.size() * sizeof(RIndex);
    IndexBufferDesc.Lifetime = ERHIResourceLifetime::Transient;
    IndexBufferDesc.Usage = ERHIBufferUsage::Index;
    IndexBufferDesc.Memory = ERHIResourceMemory::HostVisible;
    IndexBuffer.reset(GetRHI()->RHICreateBuffer(IndexBufferDesc));
    IndexBuffer->Map(IndexData.data(), IndexBufferDesc.Size, 0);
}

RPrimitive::RPrimitive(FRHIBuffer* InVertexBuffer, FRHIBuffer* InIndexBuffer)
    : VertexBuffer(InVertexBuffer), IndexBuffer(InIndexBuffer)
{
}

FRHIBuffer* RPrimitive::GetVB() const
{
    return VertexBuffer.get();
}

FRHIBuffer* RPrimitive::GetIB() const
{
    return IndexBuffer.get();
}

int32_t RPrimitive::GetIndexCount()
{
    return IndexBuffer->GetBufferDesc().Size / sizeof(uint16_t);
}