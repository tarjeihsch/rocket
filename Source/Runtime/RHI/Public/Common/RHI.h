#pragma once

#include "RHICommandList.h"

#include <cstdint>
#include <span>

struct FMeshDrawCommand;
enum class ERHIPipelineStage : uint16_t;
enum class ERHIResourceAccess : uint16_t;
enum class ERHITextureLayout : uint8_t;
struct FRHITextureView;
struct FRHITextureDesc;
struct FRHITexture;
class IRHICommandListContext;
class FRHICommandList;

enum class ERHIResourceMemory : uint8_t
{
    Unknown = 0,

    // Long-lived object with default VMA allocator
    Persistent,

    // Short-lived object with custom allocator
    Transient,

    // Memory will be managed externally
    Unmanaged,

    HostLocal,

    DeviceLocal
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
    virtual void EndFrame() = 0;

    [[nodiscard]] virtual const char* GetName() const  = 0;
    [[nodiscard]] virtual const char* GetVersion() const  = 0;
    [[nodiscard]] virtual ERHIInterfaceType  GetInterfaceType() const noexcept = 0;
    [[nodiscard]] virtual IRHI* GetNonValidationRHI() const noexcept { return const_cast<IRHI*>(this); }

    virtual uint64_t GetFrameCounter() const noexcept = 0;
    virtual void RHIWaitUntilIdle() const = 0;
    virtual FRHITexture* RHICreateTexture(FRHITextureDesc Desc) const = 0;
    virtual FRHITextureView* RHICreateTextureView(FRHITextureViewDesc TextureViewDesc, FRHITexture* Texture) const = 0;
    virtual IRHICommandListContext* GetCommandListContext() const = 0;
    virtual void RHICmdBeginRendering(FRHICommandList* RHICmdList, std::span<FRHITextureView*> Attachments) = 0;
    virtual void RHICmdPipelineBarrier(FRHICommandList* RHICmdList, FRHITexture* Texture, ERHITextureLayout SrcLayout, ERHITextureLayout DstLayout, ERHIResourceAccess SrcAccess, ERHIResourceAccess DstAccess, ERHIPipelineStage SrcStage, ERHIPipelineStage DstStage, uint32_t SrcQueueFamilyIndex = ~(0U), uint32_t DstQueueFamilyIndex = ~(0U)) = 0;
    virtual void RHICmdEndRendering(FRHICommandList* RHICmdList) = 0;
    virtual void RHICmdDraw(FRHICommandList& RHICmdList, std::span<FMeshDrawCommand> DrawCommands) = 0;
    virtual FRHITexture* RHIGetBackBufferRenderTarget() const = 0;
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