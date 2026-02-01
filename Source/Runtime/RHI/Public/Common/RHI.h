#pragma once

#include <cstdint>

#include "Assertion.h"

enum class RHIResourceAccess : uint16_t
{
    None = 0,
    ShaderSampledRead = 1 << 0,
    ShaderStorageRead = 1 << 1,
    ShaderStorageWrite = 1 << 2,
    InputAttachmentRead = 1 << 3,
    TransferRead = 1 << 4,
    TransferWrite = 1 << 5,
    ColorAttachmentWrite = 1 << 6,
    DepthStencilAttachmentWrite = 1 << 7,
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

enum class RHIInterfaceType : uint8_t
{
    None = 0,

    Vulkan,

    D3D12,

    Metal,

    WebGL
};

class RHICommandList;
class RHISurface;
class RHIPipeline;
class RHITexture;
class RHITextureView;
class RHITextureSampler;
class RHIBuffer;
class RHIBufferView;

struct RHISurfaceDesc;
struct RHIPipelineDesc;
struct RHITextureDesc;
struct RHITextureViewDesc;
struct RHITextureSamplerDesc;
struct RHIBufferDesc;
struct RHIBufferViewDesc;

class RHI
{
public:
    virtual ~RHI() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Submit(RHICommandList& CmdList) = 0;
    virtual void Reset() = 0;

    virtual const char* GetName() const  = 0;
    virtual const char* GetVersion() const  = 0;
    virtual RHIInterfaceType  GetInterfaceType() const noexcept = 0;
    virtual RHI* GetNonValidationRHI() const noexcept { return const_cast<RHI*>(this); }

    virtual void RHIWaitUntilIdle() const = 0;
    virtual uint64_t RHIGetFrameCounter() const noexcept = 0;
    
    virtual RHISurface* RHICreateSurface(const RHISurfaceDesc& Desc) = 0;
    virtual RHITexture* RHICreateTexture(const RHITextureDesc& Desc) const = 0;
    virtual RHITextureView* RHICreateTextureView(const RHITextureViewDesc& Desc, RHITexture* Texture) const = 0;
    virtual RHIBuffer* RHICreateBuffer(const RHIBufferDesc& Desc) const = 0;
    virtual RHITextureSampler* RHICreateTextureSampler(const RHITextureSamplerDesc& Desc) const = 0;
    virtual RHIPipeline* RHICreatePipeline(const RHIPipelineDesc& Desc) = 0;
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
    ensure(InRHI && InRHI->GetInterfaceType() == TRHI::StaticType);
#endif
    return static_cast<TRHI*>(InRHI->GetNonValidationRHI());
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