#pragma once

#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanResourceManager.h"

#include <string>
#include <vector>
#include <functional>

class IRHICommandListContext;
class FVulkanCommandBufferContext;

enum class EVulkanQueueType { Graphics, Compute, Transfer };

enum class ERGAccessType
{
    Read,
    Write
};

struct FRGResourceAccess
{
    ResourceHandle Handle;           // Image or Buffer, with index
    ERGAccessType     Access;             // Read or Write
    VkPipelineStageFlags2 StageMask;      // Optional: VK_PIPELINE_STAGE_*
    VkAccessFlags2         AccessMask;    // Optional: VK_ACCESS_*
    VkImageLayout          Layout;        // Only for images (optional)
};

struct FRGTextureDesc  { uint32_t W, H; VkFormat Fmt; std::string Name; VkImageLayout InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED; VkImageUsageFlags UsageFlags; VkImageAspectFlags AspectFlags; };
struct FRGBufferDesc   { size_t   Size;  VkBufferUsageFlags Flags;     };

struct FRGPass
{
    std::string                       Name;
    EVulkanQueueType                  Queue   = EVulkanQueueType::Graphics;
    std::vector<FRGResourceAccess>    Reads;
    std::vector<FRGResourceAccess>    Writes;
    std::function<void(FRHICommandList&)> RecordFn;
};

struct FRGBarrier
{
    VkImage        Image;
    VkImageLayout  OldLayout;
    VkImageLayout  NewLayout;
    VkPipelineStageFlags2 SrcStage;
    VkPipelineStageFlags2 DstStage;
};

struct FRGCompiledPayload
{
    std::vector<uint32_t>   PassIndices;        // indices into RenderGraph::Passes
    EVulkanQueueType        Queue;              // graphics / compute / transfer

    std::vector<FRGBarrier> BeginBarriers;      // to run before first pass
    std::vector<FRGBarrier> EndBarriers;        // to run after last pass
    std::vector<VkSemaphore> Wait;              // cross-queue/frame waits
    std::vector<VkSemaphore> Signal;            // cross-queue/frame signals
};

inline FRGResourceAccess WriteImage(ResourceHandle h, VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
{
    FRGResourceAccess a{};
    a.Handle        = h;
    a.Access      = ERGAccessType::Write;
    a.StageMask   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    a.AccessMask  = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    a.Layout      = layout;
    return a;
}

inline FRGResourceAccess ReadImage(ResourceHandle h, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
{
    FRGResourceAccess a{};
    a.Handle        = h;
    a.Access      = ERGAccessType::Read;
    a.StageMask   = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    a.AccessMask  = VK_ACCESS_SHADER_READ_BIT;
    a.Layout      = layout;
    return a;
}

struct FVulkanRGBuilder
{
public:
    class FVulkanRenderGraph& RenderGraph;

    FVulkanRGBuilder(FVulkanRenderGraph& InRenderGraph)
        : RenderGraph(InRenderGraph)
    {
    }

    void AddPass(std::string Name, EVulkanQueueType Queue, std::span<const FRGResourceAccess> Reads, std::span<const FRGResourceAccess> Writes, std::function<void(FRHICommandList&)>&& Lambda);

    void Import(ResourceHandle p_Handle);
};
class FVulkanDevice;

class FVulkanRenderGraph // todo: rename to VulkanFrameGraph
{
public:
    FVulkanDevice& m_Device;

    FVulkanRenderGraph(FVulkanDevice& p_Device);
    ~FVulkanRenderGraph();

    FVulkanRGBuilder& GetMutableBuilder();

    void Begin();
    void Reset();

    void Compile();
    void Execute(FVulkanCommandBufferContext& FrameCtx);

    std::vector<FRGPass> Passes;
    std::vector<ResourceHandle> Resources;
    std::vector<FRGCompiledPayload> CompiledPayloads;

    FVulkanRGBuilder Builder;
};
