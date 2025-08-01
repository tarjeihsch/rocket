#pragma once
#include <cstdint>

enum class ERHIBufferUsage : uint32_t;
enum class ERHIResourceLifetime : uint8_t;
enum class ERHIResourceMemory;
struct FRHIBuffer;

struct FBuffer
{
    FBuffer(ERHIResourceMemory Memory, ERHIBufferUsage Usage, uint32_t Stride, ERHIResourceLifetime Lifetime, uint64_t Size);
    virtual ~FBuffer();

    void UploadDataBlocking(void* Data, uint64_t Size);

private:
    FRHIBuffer* Handle;
};

struct FVertexBuffer : FBuffer
{
    FVertexBuffer(uint64_t Size);
};