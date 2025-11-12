#pragma once

#include "RHI.h"

#include <cstdint>

#include <glm/glm.hpp>

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
    virtual void Map(const void* Data, uint64_t Size, uint64_t DstOffset = 0) = 0;

    ERHIResourceAccess CurrentAccess = ERHIResourceAccess::None;
    ERHIPipelineStage CurrentPipelineStage = ERHIPipelineStage::None;
    uint32_t CurrentQueueFamilyIndex = ~(0U);
};

enum class ERHIBufferMemoryPackerAlignmentLayout
{
    // No layout
    None = 0,

    // UBO
    STD140,

    // SSBO
    STD430
};

struct FRHIBufferMemoryPackerTypeLayout
{
    // Required alignment for elements of same type (cursor must start at this value)
    uint64_t Alignment;

    // Distance between elements of same type (array stride or full size)
    uint64_t Stride;
};

class FRHIBufferMemoryPacker
{
public:
    explicit FRHIBufferMemoryPacker(ERHIBufferMemoryPackerAlignmentLayout P_AlignmentLayout)
        : M_AlignmentLayout(P_AlignmentLayout)
    {
    }

    template<typename T>
    void Write(const T& P_Object);

    template<typename T>
    void Write(const T* P_Objects, uint32_t P_Count);

    uint8_t* GetData();
    uint64_t GetSize() const;

private:
    void Align(uint64_t P_Alignment);

    ERHIBufferMemoryPackerAlignmentLayout M_AlignmentLayout;
    std::vector<uint8_t> M_Data;
    uint64_t M_Offset = 0;
};

inline uint8_t* FRHIBufferMemoryPacker::GetData()
{
    return M_Data.data();
}

inline uint64_t FRHIBufferMemoryPacker::GetSize() const
{
    return M_Offset;
}

inline void FRHIBufferMemoryPacker::Align(uint64_t P_Alignment)
{
    uint64_t Misalignment = M_Offset % P_Alignment;

    // Exceeding padding bounds, realign offset to correct padding
    if (Misalignment != 0)
    {
        uint64_t Padding = P_Alignment - Misalignment;
        M_Data.insert(M_Data.end(), Padding, 0);
        M_Offset += Padding;
    }
}

template<typename T>
constexpr FRHIBufferMemoryPackerTypeLayout GetLayout(ERHIBufferMemoryPackerAlignmentLayout P_Layout);

template<>
constexpr FRHIBufferMemoryPackerTypeLayout GetLayout<float>(ERHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    return {4, 4};
}

template<>
constexpr FRHIBufferMemoryPackerTypeLayout GetLayout<int>(ERHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    return {4, 4};
}

template<>
constexpr FRHIBufferMemoryPackerTypeLayout GetLayout<glm::vec2>(ERHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    return {8, 8};
}

template<>
constexpr FRHIBufferMemoryPackerTypeLayout GetLayout<glm::vec3>(ERHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    return {16, 16};
}

template<>
constexpr FRHIBufferMemoryPackerTypeLayout GetLayout<glm::vec4>(ERHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    return {16, 16};
}

template<>
constexpr FRHIBufferMemoryPackerTypeLayout GetLayout<glm::mat2>(ERHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    if (P_Layout == ERHIBufferMemoryPackerAlignmentLayout::STD140)
    {
        return {16, 32}; // 2 vec4 columns
    }
    else
    {
        return {8, 16};  // 2 vec2 columns
    }
}

template<>
constexpr FRHIBufferMemoryPackerTypeLayout GetLayout<glm::mat3>(ERHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    if (P_Layout == ERHIBufferMemoryPackerAlignmentLayout::STD140)
    {
        return {16, 48}; // 3 vec4 columns
    }
    else
    {
        return {16, 48}; // 3 vec3 where each aligns as vec4. NB: still 48
    }
}

template<>
constexpr FRHIBufferMemoryPackerTypeLayout GetLayout<glm::mat4>(ERHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    return {16, 64}; // 4 vec4 columns
}

template <typename T>
void FRHIBufferMemoryPacker::Write(const T& P_Object)
{
    const FRHIBufferMemoryPackerTypeLayout Layout = GetLayout<T>(M_AlignmentLayout);
    Align(Layout.Alignment);

    const uint8_t* Bytes = reinterpret_cast<const uint8_t*>(&P_Object);
    M_Data.insert(M_Data.end(), Bytes, Bytes + sizeof(T));
    M_Offset += Layout.Stride;
}

template <typename T>
void FRHIBufferMemoryPacker::Write(const T* P_Objects, uint32_t P_Count)
{
    const FRHIBufferMemoryPackerTypeLayout Layout = GetLayout<T>(M_AlignmentLayout);
    Align(Layout.Alignment);

    for (uint32_t Index = 0; Index < P_Count; Index++)
    {
        const uint8_t* Bytes = reinterpret_cast<const uint8_t*>(&P_Objects[Index]);
        M_Data.insert(M_Data.end(), Bytes, Bytes + sizeof(T));
        M_Offset += Layout.Stride;
    }
}
