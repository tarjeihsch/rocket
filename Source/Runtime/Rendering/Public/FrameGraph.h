#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

struct FRHIPipeline;
struct FRHITexture;
struct FRHITextureDesc;
struct FRHITextureView;
struct FRHITextureViewDesc;
struct FRHIBuffer;
struct FRHIBufferDesc;
class FRHICommandList;
enum class ERHIResourceAccess : uint16_t;

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

    // Clear color on attachments
    ClearColor = 1 << 6
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
    FFGTextureRef(ERHIResourceAccess InAccess, FRHITexture* InHandle);

    FRHITexture* Handle;

    // Output color buffer (color attachment)
    std::unique_ptr<FRHITextureView> RTVHandle;

    // Input for shaders (sampled textures)
    std::unique_ptr<FRHITextureView> SRVHandle;

    // Read/write in shaders (compute)
    std::unique_ptr<FRHITextureView> UAVHandle;

    // Output depth buffer (depth stencil attachment)
    std::unique_ptr<FRHITextureView> DSVHandle;
};

struct FFGBufferRef : FFGResourceRef
{
    FRHIBuffer* Handle;
};

struct FFGPipelineRef : FFGResourceRef
{
    FRHIPipeline* Handle;
};

struct FFrameGraphPass
{
    const char* DebugName;
    EFGPassFlags Flags;
    std::function<void(FRHICommandList&)> Callback;

    std::vector<FFGTextureRef> TextureReads;
    std::vector<FFGTextureRef> TextureWrites;

    std::vector<FFGBufferRef> BufferReads;
    std::vector<FFGBufferRef> BufferWrites;

    void AddTextureInput(FFGTextureRef TextureRef);
    void AddTextureOutput(FFGTextureRef TextureRef);

    void AddBufferInput(FFGBufferRef BufferRef);
    void AddBufferOutput(FFGBufferRef BufferRef);

    // todo: remove
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
    FFrameGraphPass& AddPass(const char* DebugName, EFGPassFlags Flags, std::function<void(FRHICommandList& CmdList)>&& Callback);

    void Execute();

private:
    std::vector<FFrameGraphPass> Passes;
};