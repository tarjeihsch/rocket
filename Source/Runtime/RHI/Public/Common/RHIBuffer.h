#pragma once

#include "RHI.h"

#include <glm/glm.hpp>

enum class RHIResourceAccess : uint16_t;
enum class RHIPipelineStage : uint16_t;

enum class RHIBufferUsage : uint32_t
{
    Vertex = 1 << 0,
    Index = 1 << 1,
    Uniform = 1 << 2,
    Storage = 1 << 3,
    TransferSrc = 1 << 4,
    TransferDst = 1 << 5,
    Indirect = 1 << 6,
};

inline RHIBufferUsage operator|(RHIBufferUsage Lhs, RHIBufferUsage Rhs) { return static_cast<RHIBufferUsage>(static_cast<uint8_t>(Lhs) | static_cast<uint8_t>(Rhs)); }
inline RHIBufferUsage operator&(RHIBufferUsage Lhs, RHIBufferUsage Rhs) { return static_cast<RHIBufferUsage>(static_cast<uint8_t>(Lhs) & static_cast<uint8_t>(Rhs)); }

struct RHIBufferDesc
{
    uint64_t size = 0;

    RHIResourceLifetime Lifetime = RHIResourceLifetime::Unknown;
    RHIBufferUsage Usage = RHIBufferUsage::Vertex;
    RHIResourceMemory Memory = RHIResourceMemory::Unknown;

    static RHIBufferDesc CreateStagingBuffer(uint64_t InSize)
    {
        RHIBufferDesc Desc = {};
        Desc.size = InSize;
        Desc.Lifetime = RHIResourceLifetime::Transient;
        Desc.Memory = RHIResourceMemory::HostVisible;
        Desc.Usage = RHIBufferUsage::TransferSrc;
        return Desc;
    }

    static RHIBufferDesc CreateVertexBuffer(uint64_t InSize)
    {
        RHIBufferDesc Desc = {};
        Desc.size = InSize;
        Desc.Lifetime = RHIResourceLifetime::Persistent;
        Desc.Memory = RHIResourceMemory::DeviceLocal;
        Desc.Usage = RHIBufferUsage::Vertex | RHIBufferUsage::TransferDst;
        return Desc;
    }

    static RHIBufferDesc CreateTransientVertexBuffer(uint64_t InSize)
    {
        RHIBufferDesc Desc = {};
        Desc.size = InSize;
        Desc.Lifetime = RHIResourceLifetime::Transient;
        Desc.Memory = RHIResourceMemory::HostVisible;
        Desc.Usage = RHIBufferUsage::Vertex;
        return Desc;
    }

    static RHIBufferDesc CreateIndexBuffer(uint64_t InSize)
    {
        RHIBufferDesc Desc = {};
        Desc.size = InSize;
        Desc.Lifetime = RHIResourceLifetime::Persistent;
        Desc.Memory = RHIResourceMemory::DeviceLocal;
        Desc.Usage = RHIBufferUsage::Index | RHIBufferUsage::TransferDst;
        return Desc;
    }

    static RHIBufferDesc CreateTransientIndexBuffer(uint64_t InSize)
    {
        RHIBufferDesc Desc = {};
        Desc.size = InSize;
        Desc.Lifetime = RHIResourceLifetime::Transient;
        Desc.Memory = RHIResourceMemory::HostVisible;
        Desc.Usage = RHIBufferUsage::Index;
        return Desc;
    }

    static RHIBufferDesc CreateStorageBuffer(uint64_t InSize)
    {
        RHIBufferDesc Desc = {};
        Desc.size = InSize;
        Desc.Lifetime = RHIResourceLifetime::Transient;
        Desc.Memory = RHIResourceMemory::HostVisible;
        Desc.Usage = RHIBufferUsage::Storage;
        return Desc;
    }
};

struct RHIBufferAccessState
{
    RHIResourceAccess resource_access;
    RHIPipelineStage pipeline_stage;
    uint32_t queue_family_index;

    static RHIBufferAccessState Default()
    {
        RHIBufferAccessState state = {};
        state.resource_access = RHIResourceAccess::None;
        state.pipeline_stage = RHIPipelineStage::None;
        state.queue_family_index = UINT32_MAX;
        return state;
    }

    static RHIBufferAccessState ShaderRead()
    {
        RHIBufferAccessState state = {};
        state.resource_access = RHIResourceAccess::ShaderSampledRead;
        state.pipeline_stage = RHIPipelineStage::VertexShader | RHIPipelineStage::FragmentShader | RHIPipelineStage::ComputeShader;
        state.queue_family_index = UINT32_MAX;
        return state;
    }

    static RHIBufferAccessState UnorderedAccess()
    {
        RHIBufferAccessState state = {};
        state.resource_access = RHIResourceAccess::ShaderStorageRead | RHIResourceAccess::ShaderStorageWrite;
        state.pipeline_stage = RHIPipelineStage::VertexShader | RHIPipelineStage::FragmentShader | RHIPipelineStage::ComputeShader;
        state.queue_family_index = UINT32_MAX;
        return state;
    }
};

struct RHIBuffer : public IBindable
{
    RHIBuffer(const RHIBufferDesc& in_desc, const RHIBufferAccessState& in_state)
        : desc(in_desc), state(in_state)
    {
    }

    virtual ~RHIBuffer() = default;

    virtual void* get_handle() const = 0;
    virtual void map(const void* Data, uint64_t Size, uint64_t DstOffset = 0) = 0;

    RHIBufferDesc desc;
    RHIBufferAccessState state;
};

enum class RHIBufferMemoryPackerAlignmentLayout
{
    // No layout
    None = 0,

    // UBO
    STD140,

    // SSBO
    STD430
};

struct RHIBufferMemoryPackerTypeLayout
{
    // Required alignment for elements of same type (cursor must start at this value)
    uint64_t Alignment;

    // Distance between elements of same type (array stride or full size)
    uint64_t Stride;
};

class RHIBufferMemoryPacker
{
public:
    explicit RHIBufferMemoryPacker(RHIBufferMemoryPackerAlignmentLayout P_AlignmentLayout)
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

    RHIBufferMemoryPackerAlignmentLayout M_AlignmentLayout;
    std::vector<uint8_t> M_Data;
    uint64_t M_Offset = 0;
};

inline uint8_t* RHIBufferMemoryPacker::GetData()
{
    return M_Data.data();
}

inline uint64_t RHIBufferMemoryPacker::GetSize() const
{
    return M_Offset;
}

inline void RHIBufferMemoryPacker::Align(uint64_t P_Alignment)
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
constexpr RHIBufferMemoryPackerTypeLayout GetLayout(RHIBufferMemoryPackerAlignmentLayout P_Layout);

template<>
constexpr RHIBufferMemoryPackerTypeLayout GetLayout<float>(RHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    return {4, 4};
}

template<>
constexpr RHIBufferMemoryPackerTypeLayout GetLayout<int>(RHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    return {4, 4};
}

template<>
constexpr RHIBufferMemoryPackerTypeLayout GetLayout<glm::vec2>(RHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    return {8, 8};
}

template<>
constexpr RHIBufferMemoryPackerTypeLayout GetLayout<glm::vec3>(RHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    return {16, 16};
}

template<>
constexpr RHIBufferMemoryPackerTypeLayout GetLayout<glm::vec4>(RHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    return {16, 16};
}

template<>
constexpr RHIBufferMemoryPackerTypeLayout GetLayout<glm::mat2>(RHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    if (P_Layout == RHIBufferMemoryPackerAlignmentLayout::STD140)
    {
        return {16, 32}; // 2 vec4 columns
    }
    else
    {
        return {8, 16};  // 2 vec2 columns
    }
}

template<>
constexpr RHIBufferMemoryPackerTypeLayout GetLayout<glm::mat3>(RHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    if (P_Layout == RHIBufferMemoryPackerAlignmentLayout::STD140)
    {
        return {16, 48}; // 3 vec4 columns
    }
    else
    {
        return {16, 48}; // 3 vec3 where each aligns as vec4. NB: still 48
    }
}

template<>
constexpr RHIBufferMemoryPackerTypeLayout GetLayout<glm::mat4>(RHIBufferMemoryPackerAlignmentLayout P_Layout)
{
    return {16, 64}; // 4 vec4 columns
}

template <typename T>
void RHIBufferMemoryPacker::Write(const T& P_Object)
{
    const RHIBufferMemoryPackerTypeLayout Layout = GetLayout<T>(M_AlignmentLayout);
    Align(Layout.Alignment);

    const uint8_t* Bytes = reinterpret_cast<const uint8_t*>(&P_Object);
    M_Data.insert(M_Data.end(), Bytes, Bytes + sizeof(T));
    M_Offset += Layout.Stride;
}

template <typename T>
void RHIBufferMemoryPacker::Write(const T* P_Objects, uint32_t P_Count)
{
    const RHIBufferMemoryPackerTypeLayout Layout = GetLayout<T>(M_AlignmentLayout);
    Align(Layout.Alignment);

    for (uint32_t Index = 0; Index < P_Count; Index++)
    {
        const uint8_t* Bytes = reinterpret_cast<const uint8_t*>(&P_Objects[Index]);
        M_Data.insert(M_Data.end(), Bytes, Bytes + sizeof(T));
        M_Offset += Layout.Stride;
    }
}
