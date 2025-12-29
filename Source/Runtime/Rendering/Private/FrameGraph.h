#pragma once

#include "Assertion.h"
#include "Common/RHI.h"
#include "Common/RHITexture.h"
#include "Common/RHIBuffer.h"

#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

struct RHISemaphore;
class RHISurface;
class VulkanCommandListContext;

enum class ERGTextureUsage : uint8_t
{
    Undefined = 0,

    ColorAttachment,
    DepthStencilAttachment,

    ShaderRead,
    ShaderWrite,

    CopySrc,
    CopyDst,

    Present,
};

enum class ERGAccess : uint8_t
{
    Read,
    Write,
};

struct FGTextureHandle
{
    uint32_t Id = UINT32_MAX;

    inline bool IsValid() const { return Id != UINT32_MAX; }
    inline bool operator==(const FGTextureHandle& Other) const { return Id == Other.Id; }
};

struct FGTextureViewHandle
{
    uint32_t Id = UINT32_MAX;

    inline bool IsValid() const { return Id != UINT32_MAX; }
    inline bool operator==(const FGTextureViewHandle& Other) const { return Id == Other.Id; }
};

struct FGTextureState
{
    ERHITextureLayout Layout = ERHITextureLayout::Undefined;
    ERHIResourceAccess Access = ERHIResourceAccess::None;
    ERHIPipelineStage Stage = ERHIPipelineStage::TopOfPipe;
};

struct FRGExternalTexture
{
    RHITexture* Texture = nullptr;
    FRHITextureView* DefaultView = nullptr;
    FGTextureState InitialState;
    FGTextureState FinalState;
};

struct FRGTexture
{
    std::string Name;
    FRHITextureDesc Desc;
    RHITexture* Physical = nullptr;
    FGTextureState State;
    bool bExternal = false;
};

struct FRGTextureView
{
    std::string Name;
    FRHITextureViewDesc Desc;
    FGTextureHandle Texture;
    FRHITextureView* Physical = nullptr;
    bool bExternal = false;
};

struct FRGTextureUse
{
    FGTextureHandle Texture;
    ERGTextureUsage Usage = ERGTextureUsage::Undefined;
    ERGAccess Access = ERGAccess::Read;
};

enum class ERGBufferUsage : uint8_t
{
    Undefined = 0,

    Vertex,
    Index,
    Uniform,

    StorageRead,
    StorageWrite,

    Indirect,

    CopySrc,
    CopyDst,
};

struct FRGBufferHandle
{
    uint32_t Id = UINT32_MAX;
    inline bool IsValid() const { return Id != UINT32_MAX; }
};

struct FRGBufferState
{
    ERHIResourceAccess Access = ERHIResourceAccess::None;
    ERHIPipelineStage Stage = ERHIPipelineStage::None;
    uint32_t QueueFamilyIndex = ~(0U);
};

struct FRGBuffer
{
    std::string Name;
    FRHIBufferDesc Desc = {};
    FRHIBuffer* Physical = nullptr;
    bool bExternal = false;
};

struct FRGExternalBuffer
{
    FRHIBuffer* Buffer = nullptr;
    FRGBufferState InitialState;
    FRGBufferState FinalState;
};

struct FRGBufferUse
{
    FRGBufferHandle Buffer;
    ERGBufferUsage Usage = ERGBufferUsage::Undefined;
    ERGAccess Access = ERGAccess::Read;

    uint64_t Offset = 0;
    uint64_t Size = ~0ULL;
};

struct FGPass
{
    std::string Name;

    std::vector<FRGTextureUse> Textures;
    std::vector<FRGBufferUse> Buffers;

    std::function<void(IRHICommandListContext& CmdListCtx)> Execute;

    std::vector<uint32_t> OutEdges;
    uint32_t InDegree = 0;
};

enum class ERGResourceKind : uint8_t { Texture, Buffer };

struct FDGEdgeReason
{
    enum class EType : uint8_t
    {
        ReadAfterWrite,
        WriteAfterWrite,
    };

    uint32_t FromPass;
    uint32_t ToPass;

    ERGResourceKind Kind;
    uint32_t ResourceId;

    EType Type;
};

struct FrameDependencyGraph
{
    std::vector<std::string> Nodes;
    std::vector<std::pair<uint32_t, uint32_t>> Edges;
    std::vector<uint32_t> TopologicalOrder;
    std::vector<FDGEdgeReason> EdgeReasons;
};

class FrameGraph;

class FrameGraphBuilder
{
public:
    explicit FrameGraphBuilder(FrameGraph& InGraph);

    FGTextureHandle CreateTexture(std::string_view Name, const FRHITextureDesc& Desc);
    FGTextureViewHandle CreateTextureView(std::string_view Name, const FRHITextureViewDesc& Desc, FGTextureHandle Texture);
    FRGBufferHandle CreateBuffer(std::string_view Name, const FRHIBufferDesc& Desc);

    FGTextureHandle RegisterExternalTexture(std::string_view Name, RHITexture* Texture, FRHITextureView* DefaultView, const FGTextureState& InitialState, const FGTextureState& FinalState);
    FRGBufferHandle RegisterExternalBuffer(std::string_view Name, FRHIBuffer* Buffer, const FRGBufferState& InitialState, const FRGBufferState& FinalState);

    uint32_t AddPass(std::string_view Name, const std::function<void(FrameGraphBuilder& Builder, FGPass& Pass)>& Setup, const std::function<void(IRHICommandListContext& CmdListCtx)>& Execute);

    void ReadTexture(FGPass& Pass, FGTextureHandle Texture, ERGTextureUsage Usage);
    void WriteTexture(FGPass& Pass, FGTextureHandle Texture, ERGTextureUsage Usage);

    void ReadBuffer(FGPass& Pass, FRGBufferHandle Buffer, ERGBufferUsage Usage, uint64_t Offset = 0, uint64_t Size = ~0ULL);
    void WriteBuffer(FGPass& Pass, FRGBufferHandle Buffer, ERGBufferUsage Usage, uint64_t Offset = 0, uint64_t Size = ~0ULL);

    RHITexture* ResolveTexture(FGTextureHandle Texture) const;
    FRHITextureView* ResolveTextureView(FGTextureViewHandle View) const;
    FRHIBuffer* ResolveBuffer(FRGBufferHandle Buffer) const;

private:
    FrameGraph& Graph;
};

class FrameGraph
{
public:
    FrameGraph() = default;

    FrameGraphBuilder Builder();

    void compile();
    void execute(IRHICommandListContext& ctx);
    void Reset();

    FGTextureHandle AllocTexture(std::string_view Name, const FRHITextureDesc& Desc);
    FGTextureViewHandle AllocTextureView(std::string_view Name, const FRHITextureViewDesc& Desc, FGTextureHandle Texture);
    FGTextureHandle AllocExternalTexture(std::string_view Name, RHITexture* Texture, FRHITextureView* DefaultView, const FGTextureState& InitialState, const FGTextureState& FinalState);
    FRGBufferHandle AllocBuffer(std::string_view Name, const FRHIBufferDesc& Desc);
    FRGBufferHandle AllocExternalBuffer(std::string_view Name, FRHIBuffer* Buffer, const FRGBufferState& InitialState, const FRGBufferState& FinalState);

    uint32_t AllocPass(std::string_view Name, const std::function<void(IRHICommandListContext&)>& Execute);

    [[nodiscard]] RHITexture* ResolveTexture(FGTextureHandle Texture) const;
    [[nodiscard]] FRHITextureView* ResolveTextureView(FGTextureViewHandle View) const;
    [[nodiscard]] FRHIBuffer* ResolveBuffer(FRGBufferHandle Buffer) const;

private:
    void Barrier(IRHICommandListContext& CmdListCtx, const FGPass& Pass);

    static FGTextureState GetTextureState(ERGTextureUsage Usage, ERGAccess Access);
    static FRGBufferState GetBufferState(ERGBufferUsage Usage, ERGAccess Access);

private:
    std::vector<FRGTexture> Textures;
    std::vector<FRGTextureView> Views;
    std::vector<FRGBuffer> Buffers;
    std::vector<FRGExternalTexture> ExternalTextures;
    std::vector<FRGExternalBuffer> ExternalBuffers;

    std::vector<FGPass> Passes;
    std::vector<uint32_t> CompiledPasses;

    friend class FrameGraphBuilder;
};
