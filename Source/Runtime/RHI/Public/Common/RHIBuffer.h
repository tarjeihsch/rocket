#pragma once

#include "RHI.h"

#include <cstdint>

enum class ERHIBufferUsage : uint32_t
{
    Vertex = 1 << 0,
    Index = 1 << 1,
    Uniform = 1 << 2,
    Storage = 1 << 3,
    TransferSrc = 1 << 4,
    TransferDst = 1 << 5,
    Indirect = 1 << 6,
};

inline ERHIBufferUsage operator|(ERHIBufferUsage Lhs, ERHIBufferUsage Rhs) { return static_cast<ERHIBufferUsage>(static_cast<uint8_t>(Lhs) | static_cast<uint8_t>(Rhs)); }
inline ERHIBufferUsage operator&(ERHIBufferUsage Lhs, ERHIBufferUsage Rhs) { return static_cast<ERHIBufferUsage>(static_cast<uint8_t>(Lhs) & static_cast<uint8_t>(Rhs)); }

struct FRHIBufferDesc
{
    uint64_t Size = 0;

    ERHIResourceLifetime Lifetime = ERHIResourceLifetime::Unknown;
    ERHIBufferUsage Usage = ERHIBufferUsage::Vertex;
    ERHIResourceMemory Memory = ERHIResourceMemory::Unknown;

    static FRHIBufferDesc CreateStagingBuffer(uint64_t InSize)
    {
        FRHIBufferDesc Desc = {};
        Desc.Size = InSize;
        Desc.Lifetime = ERHIResourceLifetime::Transient;
        Desc.Memory = ERHIResourceMemory::HostVisible;
        Desc.Usage = ERHIBufferUsage::TransferSrc;
        return Desc;
    }

    static FRHIBufferDesc CreateVertexBuffer(uint64_t InSize)
    {
        FRHIBufferDesc Desc = {};
        Desc.Size = InSize;
        Desc.Lifetime = ERHIResourceLifetime::Persistent;
        Desc.Memory = ERHIResourceMemory::DeviceLocal;
        Desc.Usage = ERHIBufferUsage::Vertex | ERHIBufferUsage::TransferDst;
        return Desc;
    }

    static FRHIBufferDesc CreateIndexBuffer(uint64_t InSize)
    {
        FRHIBufferDesc Desc = {};
        Desc.Size = InSize;
        Desc.Lifetime = ERHIResourceLifetime::Persistent;
        Desc.Memory = ERHIResourceMemory::DeviceLocal;
        Desc.Usage = ERHIBufferUsage::Index | ERHIBufferUsage::TransferDst;
        return Desc;
    }

    static FRHIBufferDesc CreateStorageBuffer(uint64_t InSize)
    {
        FRHIBufferDesc Desc = {};
        Desc.Size = InSize;
        Desc.Lifetime = ERHIResourceLifetime::Transient;
        Desc.Memory = ERHIResourceMemory::HostVisible;
        Desc.Usage = ERHIBufferUsage::Storage;
        return Desc;
    }
};

struct FRHIBuffer
{
    virtual ~FRHIBuffer() = default;

    virtual const FRHIBufferDesc& GetBufferDesc() const = 0;
    virtual void* GetHandle() const = 0;

    virtual uint32_t GetBindingIndex() const = 0;

    virtual void Lock(const void* Data, uint64_t Size) = 0;
    //virtual void Unlock() = 0;

    ERHIResourceAccess CurrentAccess = ERHIResourceAccess::None;
    ERHIPipelineStage CurrentPipelineStage = ERHIPipelineStage::None;
    uint32_t CurrentQueueFamilyIndex = ~(0U);
};