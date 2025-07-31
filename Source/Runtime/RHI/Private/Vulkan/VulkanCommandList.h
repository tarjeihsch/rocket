#pragma once

#include "VulkanCommandBufferContext.h"
#include "Common/RHICommandList.h"

#include <span>
#include <vulkan/vulkan_core.h>

class FVulkanCommandListContext final : public FVulkanCommandBufferContext, public IRHICommandListContext
{
public:
    FVulkanCommandListContext(FVulkanDevice& InDevice);

    virtual void RHISetScissorRect(uint32_t X, uint32_t Y) override;
    virtual void RHIDraw(std::span<FMeshDrawCommand> DrawCommands) override;
    virtual void RHIPipelineBarrier(FRHITexture *InTexture, ERHITextureLayout InOldLayout, ERHITextureLayout InNewLayout, ERHIResourceAccess InSrcAccess, ERHIResourceAccess InDstAccess, ERHIPipelineStage InSrcStage, ERHIPipelineStage InDstStage, uint32_t InSrcQueueFamilyIndex, uint32_t InDstQueueFamilyIndex) override;
    virtual void RHIBeginRendering(std::span<FRHITextureView*> TextureViews) override;
    virtual void RHIEndRendering() override;

    void RHISetMemoryBarrier(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout, VkAccessFlags p_SrcAccess, VkAccessFlags p_DstAccess,  VkPipelineStageFlags2 SrcStage, VkPipelineStageFlags2 DstStage, uint32_t p_SrcQueueFamilyIndex, uint32_t p_DstQueueFamilyIndex);
    void RHICopyBuffer(VkBuffer p_StagingBuffer, VkBuffer p_Buffer);
    void RHISetViewport(float p_X, float p_Y, float p_Width, float p_Height, float p_MinDepth, float p_MaxDepth);
    void RHISetScissor(int32_t p_OffsetX, int32_t p_OffsetY, uint32_t p_ExtentWidth, uint32_t p_ExtentHeight);
    void RHIBindPipeline(VkPipelineBindPoint pPipelineBindPoint, VkPipeline pPipeline);
    void RHIBindDescriptorSets(VkPipelineBindPoint p_PipelineBindPoint, VkPipelineLayout p_PipelineLayout, uint32_t p_FirstSet, uint32_t p_DescriptorSetCount, const VkDescriptorSet* p_DescriptorSets);
    void RHIBindVertexBuffer(VkBuffer pBuffer);
    void RHIBindIndexBuffer(VkBuffer pBuffer);
};

struct VulkanCommandCopyBuffer : TRHICommand<VulkanCommandCopyBuffer>
{
    VkBuffer m_StagingBuffer;
    VkBuffer m_Buffer;

    VulkanCommandCopyBuffer(VkBuffer p_StagingBuffer, VkBuffer p_Buffer)
        : m_StagingBuffer(p_StagingBuffer), m_Buffer(p_Buffer)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHICopyBuffer(m_StagingBuffer, m_Buffer);
    }
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
    std::span<FRHITextureView*> ImageViews;

    FVulkanCommandBeginRendering(std::span<FRHITextureView*> InViews)
        : ImageViews(InViews)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIBeginRendering(ImageViews);
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

struct VulkanCommandBindPipeline : TRHICommand<VulkanCommandBindPipeline>
{
    VkPipelineBindPoint BindPoint;
    VkPipeline Pipeline;

    VulkanCommandBindPipeline(VkPipelineBindPoint pBindPoint, VkPipeline pPipeline)
        : BindPoint(pBindPoint), Pipeline(pPipeline)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIBindPipeline(BindPoint, Pipeline);
    }
};

struct VulkanCommandBindDescriptorSets : TRHICommand<VulkanCommandBindDescriptorSets>
{
    VkPipelineBindPoint m_BindPoint;
    VkPipelineLayout m_PipelineLayout;
    uint32_t m_FirstSet;
    uint32_t m_DescriptorSetCount;
    const VkDescriptorSet *m_DescriptorSets;

    VulkanCommandBindDescriptorSets(VkPipelineBindPoint p_PipelineBindPoint, VkPipelineLayout p_PipelineLayout, uint32_t p_FirstSet, uint32_t p_DescriptorSetCount, const VkDescriptorSet *p_DescriptorSets)
        : m_BindPoint(p_PipelineBindPoint), m_PipelineLayout(p_PipelineLayout), m_FirstSet(p_FirstSet), m_DescriptorSetCount(p_DescriptorSetCount), m_DescriptorSets(p_DescriptorSets)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIBindDescriptorSets(m_BindPoint, m_PipelineLayout, m_FirstSet, m_DescriptorSetCount, m_DescriptorSets);
    }
};

struct VulkanCommandBindVertexBuffer : TRHICommand<VulkanCommandBindVertexBuffer>
{
    VkBuffer VertexBuffer;

    VulkanCommandBindVertexBuffer(VkBuffer pVertexBuffer)
        : VertexBuffer(pVertexBuffer)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIBindVertexBuffer(VertexBuffer);
    }
};

struct VulkanCommandBindIndexBuffer : TRHICommand<VulkanCommandBindIndexBuffer>
{
    VkBuffer IndexBuffer;

    VulkanCommandBindIndexBuffer(VkBuffer pIndexBuffer)
        : IndexBuffer(pIndexBuffer)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIBindIndexBuffer(IndexBuffer);
    }
};

struct FVulkanCommandDraw : TRHICommand<FVulkanCommandDraw>
{
    std::span<FMeshDrawCommand> DrawCommands;

    FVulkanCommandDraw(std::span<FMeshDrawCommand> InDrawCommands)
        : DrawCommands(InDrawCommands)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIDraw(DrawCommands);
    }
};
