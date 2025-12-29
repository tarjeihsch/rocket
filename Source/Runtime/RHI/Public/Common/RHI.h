#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <array>
#include <assert.h>

class RHISurface;
struct RHISurfaceDesc;
class RHISwapchain;
struct RRHISwapchainDesc;
struct RRHISamplerDesc;
class RRHISampler;
struct FRHIDrawCommand;
struct FRHIBufferDesc;
struct FRHIBuffer;
struct FMeshDrawCommand;
enum class ERHIPipelineStage : uint16_t;
enum class ERHIResourceAccess : uint16_t;
enum class ERHITextureLayout : uint8_t;
struct FRHITextureView;
struct FRHITextureDesc;
struct FRHITextureViewDesc;
struct RHITexture;
struct FRHIDrawParams;
class IRHICommandListContext;
class RHICommandList;
struct FRHIDrawCall;
struct FRHIPipeline;
struct FRHIPipelineDesc;

enum class ERHIResourceAccess : uint16_t
{
    None = 0,

    // GPU Shader Access
    ShaderSampledRead = 1 << 0,  // SRV: sampled image
    ShaderStorageRead = 1 << 1,  // UAV read
    ShaderStorageWrite = 1 << 2,  // UAV write
    InputAttachmentRead = 1 << 3,  // Vulkan subpass input

    // Transfer Operations
    TransferRead = 1 << 4,
    TransferWrite = 1 << 5,

    // Attachment Writes
    ColorAttachmentWrite = 1 << 6,
    DepthStencilAttachmentWrite = 1 << 7,

    // CPU Access
    HostRead = 1 << 8,
    HostWrite = 1 << 9,
};

enum class ERHIPipelineStage : uint16_t
{
    None = 0,
    TopOfPipe = 1 << 0,
    VertexShader = 1 << 1,
    FragmentShader = 1 << 2,
    ComputeShader = 1 << 3,
    Transfer = 1 << 4,
    ColorAttachment = 1 << 5,
    DepthStencil = 1 << 6,
    BottomOfPipe = 1 << 7,
};

inline ERHIPipelineStage operator|(ERHIPipelineStage Lhs, ERHIPipelineStage Rhs) { return static_cast<ERHIPipelineStage>(static_cast<uint8_t>(Lhs) | static_cast<uint8_t>(Rhs)); }
inline ERHIPipelineStage operator&(ERHIPipelineStage Lhs, ERHIPipelineStage Rhs) { return static_cast<ERHIPipelineStage>(static_cast<uint8_t>(Lhs) & static_cast<uint8_t>(Rhs)); }

enum class ERHIResourceLifetime : uint8_t
{
    Unknown = 0,

    // Long-lived object with default VMA allocator
    Persistent,

    // Short-lived object with custom allocator
    Transient,

    // Memory will be managed externally
    Unmanaged,
};

enum class ERHIResourceMemory
{
    Unknown = 0,

    // GPU-only memory; fastest access by the GPU; CPU can't map it
    DeviceLocal,

    // CPU can map this memory
    HostVisible
};

enum class ERHIInterfaceType : uint8_t
{
    None = 0,

    Vulkan,

    D3D12,

    Metal,

    WebGL
};

class IRHI
{
public:
    virtual ~IRHI() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void submit() = 0;
    virtual void reset() = 0;

    [[nodiscard]] virtual const char* GetName() const  = 0;
    [[nodiscard]] virtual const char* GetVersion() const  = 0;
    [[nodiscard]] virtual ERHIInterfaceType  GetInterfaceType() const noexcept = 0;
    [[nodiscard]] virtual IRHI* GetNonValidationRHI() const noexcept { return const_cast<IRHI*>(this); }

    virtual RHISurface* create_surface(RHISurfaceDesc& desc) = 0;
    virtual uint64_t GetFrameCounter() const noexcept = 0;
    virtual void RHIWaitUntilIdle() const = 0;
    virtual RHITexture* RHICreateTexture(FRHITextureDesc Desc) const = 0;
    virtual FRHITextureView* RHICreateTextureView(FRHITextureViewDesc TextureViewDesc, RHITexture* Texture) const = 0;
    virtual FRHIBuffer* RHICreateBuffer(FRHIBufferDesc Desc) const = 0;
    virtual RRHISampler* RHICreateSampler(RRHISamplerDesc Desc) const = 0;
    virtual void RHIUploadDataBlocking(FRHIBuffer* Buffer, void* Data, uint64_t Size) = 0;
    virtual void RHIUploadTextureBlocking(RHITexture* Texture2D, const uint8_t* Data, uint32_t Width, uint32_t Height) = 0;
    virtual FRHIPipeline* RHICreatePipelineStateObject(FRHIPipelineDesc Desc) = 0;
    virtual IRHICommandListContext* GetCommandListContext() const = 0;
    virtual void RHICmdBindPipeline(RHICommandList& RHICmdList, FRHIPipeline* Pipeline) = 0;
    virtual void RHICmdBeginRendering(RHICommandList* RHICmdList, std::span<FRHITextureView*> Attachments, FRHITextureView* DepthAttachmentView, bool bClearColor) = 0;
    virtual void RHICmdPipelineBarrier(RHICommandList* RHICmdList, RHITexture* Texture, ERHITextureLayout SrcLayout, ERHITextureLayout DstLayout, ERHIResourceAccess SrcAccess, ERHIResourceAccess DstAccess, ERHIPipelineStage SrcStage, ERHIPipelineStage DstStage, uint32_t SrcQueueFamilyIndex = ~(0U), uint32_t DstQueueFamilyIndex = ~(0U)) = 0;
    virtual void RHICmdEndRendering(RHICommandList* RHICmdList) = 0;
    virtual void RHICmdDraw(RHICommandList& RHICmdList, const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params) = 0;
    virtual void RHICmdDrawIndexed(RHICommandList& RHICmdList, const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params) = 0;
    virtual void RHICmdDrawIndexed2(RHICommandList& RHICmdList, const FRHIDrawCall& DrawCall) = 0;
    virtual void RHICmdBlitImage(RHICommandList &RHICmdList, RHITexture *Src, RHITexture *Dst) = 0;
    virtual void RHICmdSetScissor(RHICommandList& RHICmdList, int32_t X, int32_t Y, uint32_t Width, uint32_t Height) = 0;

    virtual std::shared_ptr<FRHIBuffer> RHIBindBuffer(void* Data, uint64_t Size) = 0;
    virtual std::array<uint32_t, 2> RHIGetViewportExtent() const = 0;

    // virtual void RHIUploadTextureData()
};

class IRHIModule
{
public:
    virtual ~IRHIModule() = default;

    virtual IRHI* CreateRHI() = 0;
};

extern IRHI* GRHI;
extern IRHIModule* GRHIModule;

void SetRHIModule(ERHIInterfaceType InInterfaceType);

IRHIModule* CreateVulkanRHIModule();

template<typename TRHI>
inline TRHI* CastRHI(IRHI* InRHI)
{
#ifdef _DEBUG
    assert(InRHI && InRHI->GetInterfaceType() == TRHI::StaticType);
#endif
    return static_cast<TRHI*>(InRHI->GetNonValidationRHI());
}

template<typename TRHI>
inline TRHI* GetRHI()
{
    return CastRHI<TRHI>(GRHI);
}

inline IRHI* GetRHI()
{
    return GRHI;
}
