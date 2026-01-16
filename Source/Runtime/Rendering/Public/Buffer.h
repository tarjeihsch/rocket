#pragma once
#include <cstdint>

struct RHIBufferAccessState;
class RHICommandList;
struct RHIBuffer;

class Buffer
{
public:
    RHIBuffer* GetRHIBuffer() const;

    void Memset(const void* data, size_t size, size_t offset);

protected:
    RHIBuffer* rhi_buffer;
};

class IndirectBuffer : public Buffer
{
public:
    IndirectBuffer();
};

class StorageReadBuffer : public Buffer
{
public:
};

class StorageReadWriteBuffer : public Buffer
{
public:
    StorageReadWriteBuffer();
};

class ConstantBuffer : public Buffer 
{

};

class VertexBuffer : public Buffer
{
public:
};

class IndexBuffer : public Buffer
{
public:
};