#pragma once

#include "Common/RHICommandList.h"

#include <vulkan/vulkan_core.h>

class FVulkanCommandListContext final : public IRHICommandListContext
{
public:
    explicit FVulkanCommandListContext(VkCommandBuffer InCommandBuffer)
        : CommandBuffer(InCommandBuffer)
    {
    }

    virtual void RHISetScissorRect(uint32_t X, uint32_t Y) override;

    void RHIBeginRendering(VkImageView ImageView);
    void RHIEndRendering();
    void RHISetMemoryBarrier(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout);
    void RHICopyBuffer(VkBuffer p_StagingBuffer, VkBuffer p_Buffer);
    void RHISetViewport(float p_X, float p_Y, float p_Width, float p_Height, float p_MinDepth, float p_MaxDepth);
    void RHISetScissor(int32_t p_OffsetX, int32_t p_OffsetY, uint32_t p_ExtentWidth, uint32_t p_ExtentHeight);
    void RHIBindPipeline(VkPipelineBindPoint pPipelineBindPoint, VkPipeline pPipeline);
    void RHIBindDescriptorSets(VkPipelineBindPoint p_PipelineBindPoint, VkPipelineLayout p_PipelineLayout, uint32_t p_FirstSet, uint32_t p_DescriptorSetCount, const VkDescriptorSet* p_DescriptorSets);
    void RHIBindVertexBuffer(VkBuffer pBuffer);
    void RHIBindIndexBuffer(VkBuffer pBuffer);

private:
    VkCommandBuffer CommandBuffer;
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

struct FVulkanCommandSetMemoryBarrier : TRHICommand<FVulkanCommandSetMemoryBarrier>
{
    VkImage Image;
    VkImageLayout OldLayout;
    VkImageLayout NewLayout;

    FVulkanCommandSetMemoryBarrier(VkImage InImage, VkImageLayout InOldLayout, VkImageLayout InNewLayout)
        : Image(InImage), OldLayout(InOldLayout), NewLayout(InNewLayout)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHISetMemoryBarrier(Image, OldLayout, NewLayout);
    }
};

struct FVulkanCommandBeginRendering : TRHICommand<FVulkanCommandBeginRendering>
{
    VkImageView ImageView;

    FVulkanCommandBeginRendering(VkImageView InView)
        : ImageView(InView)
    {
    }

    void Execute_Internal(IRHICommandListContext& CmdListCtx)
    {
        auto& Ctx = static_cast<FVulkanCommandListContext&>(CmdListCtx);
        Ctx.RHIBeginRendering(ImageView);
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