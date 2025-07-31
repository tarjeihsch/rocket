#pragma once

#include "RHI.h"
#include "RHICommandList.h"
#include "RHITexture.h"

#include <functional>
#include <vector>
#include <cstdint>
#include <memory>

struct FRHITextureViewDesc;
struct FRHITextureDesc;
struct FRHITexture;
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

inline EFGPassFlags operator|(EFGPassFlags Lhs, EFGPassFlags Rhs) { return static_cast<EFGPassFlags>(static_cast<uint8_t>(Lhs) | static_cast<uint8_t>(Rhs)); }
inline EFGPassFlags operator&(EFGPassFlags Lhs, EFGPassFlags Rhs) { return static_cast<EFGPassFlags>(static_cast<uint8_t>(Lhs) & static_cast<uint8_t>(Rhs)); }

struct FFGResourceRef
{
    explicit FFGResourceRef(ERHIResourceAccess InAccess)
        : TargetAccess(InAccess)
    {
    }

    ERHIResourceAccess TargetAccess;
};

struct FFGTextureRef : FFGResourceRef
{
    FFGTextureRef(ERHIResourceAccess InAccess, FRHITexture* InHandle)
        : FFGResourceRef(InAccess), Handle(InHandle)
    {
        RTVHandle.reset(GetRHI()->RHICreateTextureView(FRHITextureViewDesc::Create2D(ERHITextureFormat::B8G8R8A8_SRGB, ERHITextureAspect::Color), Handle));
        SRVHandle.reset(GetRHI()->RHICreateTextureView(FRHITextureViewDesc::Create2D(ERHITextureFormat::B8G8R8A8_SRGB, ERHITextureAspect::Color), Handle));
        UAVHandle.reset(GetRHI()->RHICreateTextureView(FRHITextureViewDesc::Create2D(ERHITextureFormat::B8G8R8A8_SRGB, ERHITextureAspect::Color), Handle));
    }

    FRHITexture* Handle;

    // Output color buffer
    std::unique_ptr<FRHITextureView> RTVHandle;

    // Input for shaders
    std::unique_ptr<FRHITextureView> SRVHandle;

    // Read/write in shaders
    std::unique_ptr<FRHITextureView> UAVHandle;
};

struct FFGBufferRef : FFGResourceRef
{
    //FRHIBuffer* Handle;
};

struct FFrameGraphPass
{
    const char* DebugName;
    EFGPassFlags Flags;
    std::function<void(FRHICommandList&)> Callback;

    std::vector<FFGTextureRef> TextureReads;
    std::vector<FFGTextureRef> TextureWrites;

    void AddTextureInput();
    void AddTextureOutput();

    void AddColorAttachment(FRHITexture* Resource);
    void AddInputAttachment(FRHITexture* Resource);
    void AddSampledTexture(FRHITexture* Resource);
    void AddDepthAttachment(FRHITexture* Resource);
    void AddStorageImage(FRHITexture* Resource);
    void AddTransferDst(FRHITexture* Resource);
    void AddTransferSrc(FRHITexture* Resource);
    void AddPresentSrc(FRHITexture* Resource);
};

struct FFrameGraphBuilder
{
    FFrameGraphBuilder();
    ~FFrameGraphBuilder();

    FFrameGraphPass& AddPass(const char* DebugName, EFGPassFlags Flags, std::function<void(FRHICommandList& CmdList)>&& Callback);

    void Execute();

private:
    void AddFirstTextureTransition(FFGTextureRef TextureRef);
    void AddLastTextureTransition(FFGTextureRef TextureRef);

private:
    std::vector<FFrameGraphPass> Passes;
};