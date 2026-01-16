#include "Buffer.h"

#include "Common/RHI.h"
#include "Common/RHIBuffer.h"
#include "Common/RHICommandList.h"

IndirectBuffer::IndirectBuffer()
{
    RHIBufferDesc buffer_desc = {};
    buffer_desc.Lifetime = RHIResourceLifetime::Transient;
    buffer_desc.Memory = RHIResourceMemory::DeviceLocal;
    buffer_desc.Usage = RHIBufferUsage::Indirect | RHIBufferUsage::Storage;
    rhi_buffer = GetRHI()->RHICreateBuffer(buffer_desc);
}

StorageReadWriteBuffer::StorageReadWriteBuffer()
{
    RHIBufferDesc buffer_desc = {};
    buffer_desc.Lifetime = RHIResourceLifetime::Transient;
    buffer_desc.Memory = RHIResourceMemory::DeviceLocal;
    buffer_desc.Usage = RHIBufferUsage::Indirect | RHIBufferUsage::Storage;
    rhi_buffer = GetRHI()->RHICreateBuffer(buffer_desc);
}

RHIBuffer * Buffer::GetRHIBuffer() const
{
    return rhi_buffer;
}

void Buffer::Memset(const void* data, size_t size, size_t offset)
{
    GetRHIBuffer()->Map(data, size, offset);
}
