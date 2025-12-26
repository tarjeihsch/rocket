#pragma once

#include <vulkan/vulkan_core.h>

#include "Common/RHIDrawCommand.h"
#include "Common/RHICommandList.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBufferContext.h"

struct FRHIPipeline;
struct FMeshDrawCommand;

class FVulkanCommandListContext final : public FVulkanCommandBufferContext, public IRHICommandListContext
{
public:
    FVulkanCommandListContext(FVulkanDevice& InDevice);

    virtual void RHIBindPipeline(FRHIPipeline* Pipeline) override;
    virtual void RHISetScissorRect(int32_t X, int32_t Y, uint32_t Width, uint32_t Height) override;
    virtual void RHIDraw(const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params) override;
    virtual void RHIDrawIndexed(const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& InDrawParams) override;
    virtual void RHIDrawIndexed2(const FRHIDrawCall& DrawCall) override;
    virtual void RHIPipelineBarrier(FRHITexture *InTexture, ERHITextureLayout InOldLayout, ERHITextureLayout InNewLayout, ERHIResourceAccess InSrcAccess, ERHIResourceAccess InDstAccess, ERHIPipelineStage InSrcStage, ERHIPipelineStage InDstStage, uint32_t InSrcQueueFamilyIndex, uint32_t InDstQueueFamilyIndex) override;
    virtual void RHIBeginRendering(std::span<FRHITextureView*> ColorAttachmentsView, FRHITextureView* DepthStencilAttachmentView, bool bClearColor) override;
    virtual void RHIEndRendering() override;
    virtual void RHICopyBuffer(FRHIBuffer* Src, FRHIBuffer* Dst, uint32_t InSrcOffset, uint32_t InDstOffset, uint32_t InSize) override;
    virtual void RHICmdBlitImage(FRHITexture* Src, FRHITexture* Dst) override;

    void RHICopyBufferToImage(FRHIBuffer *SrcBuffer, FRHITexture *DstTexture, VkImageLayout DstImageLayout, uint32_t Width, uint32_t Height, uint32_t MipLevel, uint32_t BaseArrayLayer, uint32_t LayerCount, uint64_t BufferOffset);
    void RHISetMemoryBarrier(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout, VkAccessFlags p_SrcAccess, VkAccessFlags p_DstAccess,  VkPipelineStageFlags2 SrcStage, VkPipelineStageFlags2 DstStage, uint32_t p_SrcQueueFamilyIndex, uint32_t p_DstQueueFamilyIndex);
    void RHISetViewport(float p_X, float p_Y, float p_Width, float p_Height, float p_MinDepth, float p_MaxDepth);
};

struct FVulkanCommandPipelineBarrier : TRHICommand<FVulkanCommandPipelineBarrier>
{
    FRHITexture* Texture;
    ERHITextureLayout OldLayout;
    ERHITextureLayout NewLayout;
    ERHIResourceAccess SrcAccess;
    ERHIResourceAccess DstAccess;
    ERHIPipelineStage SrcStage;
    ERHIPipelineStage DstStage;
    uint32_t SrcQueueFamilyIndex;
    uint32_t DstQueueFamilyIndex;

    FVulkanCommandPipelineBarrier(FRHITexture* InTexture, ERHITextureLayout InOldLayout, ERHITextureLayout InNewLayout, ERHIResourceAccess InSrcAccess, ERHIResourceAccess InDstAccess,ERHIPipelineStage InSrcStage, ERHIPipelineStage InDstStage, uint32_t InSrcQueueFamilyIndex = ~(0U), uint32_t InDstQueueFamilyIndex= ~(0U))
        : Texture(InTexture),
        OldLayout(InOldLayout),
        NewLayout(InNewLayout),
        SrcAccess(InSrcAccess),
        DstAccess(InDstAccess),
        SrcStage(InSrcStage),
        DstStage(InDstStage),
        SrcQueueFamilyIndex(InSrcQueueFamilyIndex),
        DstQueueFamilyIndex(InDstQueueFamilyIndex)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIPipelineBarrier(Texture, OldLayout, NewLayout, SrcAccess, DstAccess, SrcStage, DstStage, SrcQueueFamilyIndex, DstQueueFamilyIndex);
    }
};

struct FVulkanCommandBeginRendering : TRHICommand<FVulkanCommandBeginRendering>
{
    std::span<FRHITextureView*> ColorAttachmentsView;
    FRHITextureView* DepthAttachmentView;
    bool bClearColor;

    FVulkanCommandBeginRendering(std::span<FRHITextureView*> InViews, FRHITextureView* InDepthAttachmentView, bool InClearColor)
        : ColorAttachmentsView(InViews), DepthAttachmentView(InDepthAttachmentView), bClearColor(InClearColor)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIBeginRendering(ColorAttachmentsView, DepthAttachmentView, bClearColor);
    }
};

struct FVulkanCommandEndRendering : TRHICommand<FVulkanCommandEndRendering>
{
    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIEndRendering();
    }
};

struct VulkanCommandSetViewport : TRHICommand<VulkanCommandSetViewport>
{
    float m_Width;
    float m_Height;
    float m_MinDepth;
    float m_MaxDepth;

    VulkanCommandSetViewport(float p_Width, float p_Height, float p_MinDepth, float p_MaxDepth)
        : m_Width(p_Width), m_Height(p_Height), m_MinDepth(p_MinDepth), m_MaxDepth(p_MaxDepth)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHISetViewport(0, 0, m_Width, m_Height, m_MinDepth, m_MaxDepth);
    }
};

struct FVulkanCommandBindPipeline : TRHICommand<FVulkanCommandBindPipeline>
{
    FRHIPipeline* Pipeline;

    FVulkanCommandBindPipeline(FRHIPipeline* InPipeline)
        : Pipeline(InPipeline)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIBindPipeline(Pipeline);
    }
};

struct FVulkanCommandDraw : TRHICommand<FVulkanCommandDraw>
{
    FRHIDrawCommand DrawCommand;
    FRHIDrawParams DrawParams;

    FVulkanCommandDraw(const FRHIDrawCommand& InDrawCommand, const FRHIDrawParams& InDrawParams)
        : DrawCommand(InDrawCommand), DrawParams(InDrawParams)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIDraw(DrawCommand, DrawParams);
    }
};

struct FVulkanCommandDrawIndexed : TRHICommand<FVulkanCommandDrawIndexed>
{
    FRHIDrawCommand DrawCommand;
    FRHIDrawParams DrawParams;

    FVulkanCommandDrawIndexed(const FRHIDrawCommand& InDrawCommand, const FRHIDrawParams& InDrawParams)
        : DrawCommand(InDrawCommand), DrawParams(InDrawParams)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIDrawIndexed(DrawCommand, DrawParams);
    }
};

struct FVulkanCommandDrawIndexed2 : TRHICommand<FVulkanCommandDrawIndexed2>
{
    FRHIDrawCall DrawCall;

    FVulkanCommandDrawIndexed2(const FRHIDrawCall& InDrawCall)
        : DrawCall(InDrawCall)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIDrawIndexed2(DrawCall);
    }
};


struct FVulkanCommandCopyBuffer : TRHICommand<FVulkanCommandCopyBuffer>
{
    FRHIBuffer* SrcBuffer;
    FRHIBuffer* DstBuffer;
    uint32_t SrcOffset;
    uint32_t DstOffset;
    uint32_t Size;

    FVulkanCommandCopyBuffer(FRHIBuffer* InSrcBuffer, FRHIBuffer* InDstBuffer, uint32_t InSrcOffset, uint32_t InDstOffset, uint32_t InSize)
        : SrcBuffer(InSrcBuffer), DstBuffer(InDstBuffer), SrcOffset(InSrcOffset), DstOffset(InDstOffset), Size(InSize)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHICopyBuffer(SrcBuffer, DstBuffer, SrcOffset, DstOffset, Size);
    }
};

struct FVulkanCommandCopyBufferToImage : TRHICommand<FVulkanCommandCopyBufferToImage>
{
    FRHIBuffer* SrcBuffer;
    FRHITexture* DstTexture;
    VkImageLayout DstImageLayout;
    uint32_t Width;
    uint32_t Height;
    uint32_t MipLevel;
    uint32_t BaseArrayLayer;
    uint32_t LayerCount;
    uint64_t BufferOffset;

    FVulkanCommandCopyBufferToImage(FRHIBuffer* InSrcBuffer, FRHITexture* InDstBuffer, VkImageLayout InDstImageLayout, uint32_t InWidth, uint32_t InHeight, uint32_t InMipLevel = 0, uint32_t InBaseArrayLayer = 0, uint32_t InLayerCount = 1, uint64_t InBufferOffset = 0)
        : SrcBuffer(InSrcBuffer), DstTexture(InDstBuffer), DstImageLayout(InDstImageLayout), Width(InWidth), Height(InHeight), MipLevel(InMipLevel), BaseArrayLayer(InBaseArrayLayer), LayerCount(InLayerCount), BufferOffset(InBufferOffset)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHICopyBufferToImage(SrcBuffer, DstTexture, DstImageLayout, Width, Height, MipLevel, BaseArrayLayer, LayerCount, BufferOffset);
    }
};


struct FVulkanCommandBlitImage : TRHICommand<FVulkanCommandBlitImage>
{
    FRHITexture *Src;
    FRHITexture *Dst;

    FVulkanCommandBlitImage(FRHITexture* InSrc, FRHITexture* InDst)
        : Src(InSrc), Dst(InDst)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHICmdBlitImage(Src, Dst);
    }
};

struct FVulkanCommandSetScissor : TRHICommand<FVulkanCommandSetScissor>
{
    int32_t X;
    int32_t Y;
    uint32_t Width;
    uint32_t Height;

    FVulkanCommandSetScissor(int32_t InX, int32_t InY, uint32_t InWidth, uint32_t InHeight)
        : X(InX), Y(InY), Width(InWidth), Height(InHeight)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHISetScissorRect(X, Y, Width, Height);
    }
};
