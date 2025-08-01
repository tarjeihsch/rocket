#include "Buffer.h"
#include "Common/Buffer.h"

#include "Common/RHI.h"

FBuffer::FBuffer(ERHIResourceMemory Memory, ERHIBufferUsage Usage, uint32_t Stride, ERHIResourceLifetime Lifetime, uint64_t Size)
{
    FRHIBufferDesc Desc;
    Desc.Memory = Memory;
    Desc.TotalSize = Size;
    Desc.Usage = Usage;
    Desc.Stride = Stride;
    Desc.Lifetime = Lifetime;

    Handle = GetRHI()->RHICreateBuffer(Desc);
}

FBuffer::~FBuffer()
{
    delete Handle;
}

void FBuffer::UploadDataBlocking(void *Data, uint64_t Size)
{
    GetRHI()->RHIUploadBufferData(Handle, Data, Size);
}

FVertexBuffer::FVertexBuffer(uint64_t Size)
    : FBuffer(ERHIResourceMemory::DeviceLocal, ERHIBufferUsage::Vertex | ERHIBufferUsage::TransferDst, 16, ERHIResourceLifetime::Persistent, Size)
{
}