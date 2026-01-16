#pragma once

#include <cstdint>

#include "Assertion.h"

class RHISurface;
struct RHISurfaceDesc;
class RHISwapchain;
struct RRHISwapchainDesc;
struct RHISamplerDesc;
class RHISampler;
struct FRHIDrawCommand;
struct RHIBufferDesc;
struct RHIBuffer;
struct FMeshDrawCommand;
enum class RHIPipelineStage : uint16_t;
enum class RHIResourceAccess : uint16_t;
enum class RHITextureLayout : uint8_t;
struct RHITextureView;
struct RHITextureDesc;
struct RHITextureViewDesc;
struct RHITexture;
struct FRHIDrawParams;
class RHICommandListContext;
class RHICommandList;
struct FRHIDrawCall;
struct RHIPipeline;
struct RHIPipelineDesc;

class IBindable
{
    virtual uint32_t GetBindingIndex() const = 0;
};

enum class RHIResourceAccess : uint16_t
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

    IndirectCommandRead = 1 << 10,
};

enum class RHIPipelineStage : uint16_t
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
    DrawIndirect = 1 << 8,
    AllCommands = 1 << 9
};

inline RHIResourceAccess operator|(RHIResourceAccess Lhs, RHIResourceAccess Rhs) { return static_cast<RHIResourceAccess>(static_cast<uint16_t>(Lhs) | static_cast<uint16_t>(Rhs)); }
inline RHIResourceAccess operator&(RHIResourceAccess Lhs, RHIResourceAccess Rhs) { return static_cast<RHIResourceAccess>(static_cast<uint16_t>(Lhs) & static_cast<uint16_t>(Rhs)); }

inline RHIPipelineStage operator|(RHIPipelineStage Lhs, RHIPipelineStage Rhs) { return static_cast<RHIPipelineStage>(static_cast<uint16_t>(Lhs) | static_cast<uint16_t>(Rhs)); }
inline RHIPipelineStage operator&(RHIPipelineStage Lhs, RHIPipelineStage Rhs) { return static_cast<RHIPipelineStage>(static_cast<uint16_t>(Lhs) & static_cast<uint16_t>(Rhs)); }

enum class RHIResourceLifetime : uint8_t
{
    Unknown = 0,

    // Long-lived object with default VMA allocator
    Persistent,

    // Short-lived object with custom allocator
    Transient,

    // Memory will be managed externally
    Unmanaged,
};

enum class RHIResourceMemory
{
    Unknown = 0,

    // GPU-only memory; fastest access by the GPU; CPU can't map it
    DeviceLocal,

    // GPU-memory; CPU can map this memory - slower access
    HostVisible
};

enum class RHIInterfaceType : uint8_t
{
    None = 0,

    Vulkan,

    D3D12,

    Metal,

    WebGL
};

class RHI
{
public:
    virtual ~RHI() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Submit(RHICommandList&) = 0;
    virtual void Reset() = 0;

    [[nodiscard]] virtual const char* get_name() const  = 0;
    [[nodiscard]] virtual const char* get_version() const  = 0;
    [[nodiscard]] virtual RHIInterfaceType  get_interface_type() const noexcept = 0;
    [[nodiscard]] virtual RHI* get_non_validation_rhi() const noexcept { return const_cast<RHI*>(this); }

    virtual void rhi_wait_until_idle() const = 0;
    virtual uint64_t rhi_get_frame_counter() const noexcept = 0;
    virtual RHISurface* CreateSurface(RHISurfaceDesc desc) = 0;
    virtual RHITexture* RHICreateTexture(RHITextureDesc Desc) const = 0;
    virtual RHITextureView* RHICreateTextureView(RHITextureViewDesc TextureViewDesc, RHITexture* Texture) const = 0;
    virtual RHIBuffer* RHICreateBuffer(RHIBufferDesc Desc) const = 0;
    virtual RHISampler* RHICreateSampler(RHISamplerDesc Desc) const = 0;
    virtual RHIPipeline* RHICreatePipeline(RHIPipelineDesc Desc) = 0;
};

class RHIModule
{
public:
    virtual ~RHIModule() = default;

    virtual RHI* CreateRHI() = 0;
};

extern RHI* g_rhi;
extern RHIModule* g_rhi_module;

void SetRHIModule(RHIInterfaceType InInterfaceType);

RHIModule* create_vulkan_rhi_module();

template<typename TRHI>
inline TRHI* CastRHI(RHI* InRHI)
{
#ifdef RK_DEBUG
    ensure(InRHI && InRHI->get_interface_type() == TRHI::StaticType);
#endif
    return static_cast<TRHI*>(InRHI->get_non_validation_rhi());
}

template<typename TRHI>
inline TRHI* GetRHI()
{
    return CastRHI<TRHI>(g_rhi);
}

inline RHI* GetRHI()
{
    return g_rhi;
}