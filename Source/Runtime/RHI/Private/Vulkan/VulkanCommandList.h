#pragma once

#include "Common/RHICommandList.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBufferContext.h"

#include <span>
#include <vulkan/vulkan_core.h>

#include "Common/RHIDrawCommand.h"

struct FRHIPipeline;
struct FMeshDrawCommand;

class FVulkanCommandListContext final : public FVulkanCommandBufferContext, public IRHICommandListContext
{
public:
    FVulkanCommandListContext(FVulkanDevice& InDevice);

    virtual void RHIBindPipeline(FRHIPipeline* Pipeline) override;
    virtual void RHISetScissorRect(uint32_t X, uint32_t Y) override;
    virtual void RHIDraw(const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params) override;
    virtual void RHIDrawIndexed(const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& InDrawParams) override;
    virtual void RHIPipelineBarrier(FRHITexture *InTexture, ERHITextureLayout InOldLayout, ERHITextureLayout InNewLayout, ERHIResourceAccess InSrcAccess, ERHIResourceAccess InDstAccess, ERHIPipelineStage InSrcStage, ERHIPipelineStage InDstStage, uint32_t InSrcQueueFamilyIndex, uint32_t InDstQueueFamilyIndex) override;
    virtual void RHIBeginRendering(std::span<FRHITextureView*> ColorAttachmentsView, FRHITextureView* DepthStencilAttachmentView) override;
    virtual void RHIEndRendering() override;
    virtual void RHICopyBuffer(FRHIBuffer* Src, FRHIBuffer* Dst, uint32_t InSrcOffset, uint32_t InDstOffset, uint32_t InSize) override;
    virtual void RHICmdBlitImage(FRHITexture* Src, FRHITexture* Dst) override;

    void RHISetMemoryBarrier(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout, VkAccessFlags p_SrcAccess, VkAccessFlags p_DstAccess,  VkPipelineStageFlags2 SrcStage, VkPipelineStageFlags2 DstStage, uint32_t p_SrcQueueFamilyIndex, uint32_t p_DstQueueFamilyIndex);
    void RHISetViewport(float p_X, float p_Y, float p_Width, float p_Height, float p_MinDepth, float p_MaxDepth);
    void RHISetScissor(int32_t p_OffsetX, int32_t p_OffsetY, uint32_t p_ExtentWidth, uint32_t p_ExtentHeight);
    void RHIBindDescriptorSets(VkPipelineBindPoint p_PipelineBindPoint, VkPipelineLayout p_PipelineLayout, uint32_t p_FirstSet, uint32_t p_DescriptorSetCount, const VkDescriptorSet* p_DescriptorSets);
    void RHIBindVertexBuffer(VkBuffer pBuffer);
    void RHIBindIndexBuffer(VkBuffer pBuffer);
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

    FVulkanCommandBeginRendering(std::span<FRHITextureView*> InViews, FRHITextureView* InDepthAttachmentView)
        : ColorAttachmentsView(InViews), DepthAttachmentView(InDepthAttachmentView)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIBeginRendering(ColorAttachmentsView, DepthAttachmentView);
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

struct VulkanCommandSetScissor : TRHICommand<VulkanCommandSetScissor>
{
    int32_t m_OffsetX;
    int32_t m_OffsetY;
    uint32_t m_ExtentWidth;
    uint32_t m_ExtentHeight;

    VulkanCommandSetScissor(int32_t p_OffsetX, int32_t p_OffsetY, uint32_t p_ExtentWidth, uint32_t p_ExtentHeight)
            : m_OffsetX(p_OffsetX), m_OffsetY(p_OffsetY), m_ExtentWidth(p_ExtentWidth), m_ExtentHeight(p_ExtentHeight)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHISetScissor(m_OffsetX, m_OffsetY, m_ExtentWidth, m_ExtentHeight);
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