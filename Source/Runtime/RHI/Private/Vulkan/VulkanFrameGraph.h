#pragma once

#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan_core.h"

#include <functional>
#include <vector>

struct FVulkanTextureView;
struct FRHITextureViewDesc;
struct FVulkanTexture;
struct FRHITextureDesc;
struct FRHITexture;
class FVulkanDevice;
class FRHICommandList;

enum class EFGPassFlags : uint8_t
{
    None = 0,

    // Executes rasterizer pass on graphics queue
    Graphics = 1 << 0,

    // Executes compute pass on graphics queue
    Compute = 1 << 1,

    // Executes transfer pass on graphics queue
    Copy = 1 << 2,

    // Executes compute pass on compute queue
    AsyncCompute = 1 << 3,

    // Executes transfer pass on compute queue
    AsyncCopy = 1 << 4,

    // Use dynamic rendering
    Dynamic = 1 << 5,
};

enum class EFGAccessType
{
    SampledRead,

    InputAttachmentRead,

    ColorAttachmentWrite,

    StorageWrite,

    TransferWrite,

    TransferRead,

    DepthStencilWrite,

    Present
};

inline EFGPassFlags operator|(EFGPassFlags Lhs, EFGPassFlags Rhs) { return static_cast<EFGPassFlags>(static_cast<uint8_t>(Lhs) | static_cast<uint8_t>(Rhs)); }
inline EFGPassFlags operator&(EFGPassFlags Lhs, EFGPassFlags Rhs) { return static_cast<EFGPassFlags>(static_cast<uint8_t>(Lhs) & static_cast<uint8_t>(Rhs)); }

enum class EFGResourceType : uint8_t
{
    Unknown = 0,

    Texture,

    Buffer,

    Semaphore
};

struct FFGResourceRef
{
    uint8_t bTransient : 1;

    EFGAccessType AccessType;

    EFGResourceType ResourceType;

    inline bool IsTexture() const
    {
        return ResourceType == EFGResourceType::Texture;
    }

    inline bool IsBuffer() const
    {
        return ResourceType == EFGResourceType::Buffer;
    }
};

struct FFGTextureRef : FFGResourceRef
{
    FFGTextureRef();

    FVulkanTexture* Handle;
    FVulkanTextureView* View; // Is this even needed? Can we just have a helper to quickly generate a image view on demand?

    VkImageLayout CurrentImageLayout;
    VkAccessFlags CurrentAccessFlags;
    VkPipelineStageFlags CurrentPipelineStageFlags;
};

struct FFGBufferRef : FFGResourceRef
{
};

using FFGPassRef = uint32_t;

struct FFGResourceUsage
{
    FFGResourceRef* Resource;
    EFGAccessType AccessType;
};

struct FFGPass
{
    const char* DebugName;
    std::function<void(FRHICommandList&)> Callback;
    EFGPassFlags Flags;

    std::vector<FFGResourceUsage> Reads;
    std::vector<FFGResourceUsage> Writes;

    void AddColorAttachment(FFGResourceRef* Resource);
    void AddInputAttachment(FFGResourceRef* Resource);
    void AddSampledTexture(FFGResourceRef* Resource);
    void AddDepthAttachment(FFGResourceRef* Resource);
    void AddStorageImage(FFGResourceRef* Resource);
    void AddTransferDst(FFGResourceRef* Resource);
    void AddTransferSrc(FFGResourceRef* Resource);
    void AddPresentSrc(FFGResourceRef* Resource);
};

class FFrameGraph
{
public:
    FFrameGraph(FVulkanDevice& p_Device);
    ~FFrameGraph();

    FFGPass& AddPass(const char* DebugName, std::function<void(FRHICommandList& CmdList)>&& Callback);

    void Analyze();
    void Execute();

    FFGTextureRef CreateTexture(FRHITextureDesc InTextureDesc, FRHITextureViewDesc InTextureViewDesc);
    FFGTextureRef ImportTexture(FVulkanTexture* InTexture);

    //FFGBufferRef CreateBuffer(FRHIBufferDesc InBufferDesc);
    //FFGBufferRef ImportBuffer(FVulkanBuffer* InBuffer);

private:
    std::vector<FFGPass> Passes;
    std::vector<FVulkanTextureView> TextureViews;

private:
    FVulkanDevice& m_Device;
};