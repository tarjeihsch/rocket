#pragma once

#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanResourceManager.h"

#include <string>
#include <vector>
#include <functional>

class IRHICommandListContext;
class FVulkanCommandBufferContext;

enum class FrameGraphSubmissionQueue { Graphics, Compute, Transfer };

enum class BarrierType { Image, Buffer };

enum class ERGAccessType
{
    Read,
    Write
};

struct FrameGraphResourceAccess
{
    ResourceHandle Handle;
    ERGAccessType Access;             // Read or Write
    VkPipelineStageFlags2 StageMask;      // Optional: VK_PIPELINE_STAGE_*
    VkAccessFlags2 AccessMask;    // Optional: VK_ACCESS_*
    VkImageLayout Layout;        // Only for images (optional)
};

struct FrameGraphPass
{
    std::string Name;
    FrameGraphSubmissionQueue Queue = FrameGraphSubmissionQueue::Graphics;
    std::vector<FrameGraphResourceAccess> Inputs;
    std::vector<FrameGraphResourceAccess> Outputs;
    std::function<void(FRHICommandList&)> Callback;
};

struct FrameGraphBarrier
{
    BarrierType type;

    // COMMON:
    VkPipelineStageFlags2   SrcStage = {};
    VkPipelineStageFlags2   DstStage = {};
    FrameGraphSubmissionQueue srcQueue;
    FrameGraphSubmissionQueue dstQueue;

    // IMAGE‑specific:
    VkImage                 Image{};
    VkImageLayout           OldLayout{};
    VkImageLayout           NewLayout{};

    // BUFFER‑specific:
    VkBuffer                buffer{};
    VkDeviceSize            offset = 0;
    VkDeviceSize            size   = VK_WHOLE_SIZE;
    VkAccessFlags2          srcAccess{};
    VkAccessFlags2          dstAccess{};
};

struct FrameGraphPayload
{
    std::vector<uint32_t> PassIndices;        // indices into RenderGraph::Passes
    FrameGraphSubmissionQueue Queue;
    std::vector<FrameGraphBarrier> BeginBarriers;      // to run before first pass
    std::vector<FrameGraphBarrier> EndBarriers;        // to run after last pass
    std::vector<VkSemaphore> Wait;              // cross-queue/frame waits
    std::vector<VkSemaphore> Signal;            // cross-queue/frame signals
};

inline FrameGraphResourceAccess WriteImage(ResourceHandle p_Handle, VkImageLayout p_ImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
{
    FrameGraphResourceAccess ResourceAccess = {};
    ResourceAccess.Handle = p_Handle;
    ResourceAccess.Access = ERGAccessType::Write;
    ResourceAccess.StageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    ResourceAccess.AccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    ResourceAccess.Layout = p_ImageLayout;
    return ResourceAccess;
}

inline FrameGraphResourceAccess ReadImage(ResourceHandle p_Handle, VkImageLayout p_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
{
    FrameGraphResourceAccess ResourceAccess = {};
    ResourceAccess.Handle = p_Handle;
    ResourceAccess.Access = ERGAccessType::Read;
    ResourceAccess.StageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    ResourceAccess.AccessMask = VK_ACCESS_SHADER_READ_BIT;
    ResourceAccess.Layout = p_ImageLayout;
    return ResourceAccess;
}

class FVulkanDevice;

class VulkanFrameGraph
{
public:
    FVulkanDevice& m_Device;

    VulkanFrameGraph(FVulkanDevice& p_Device);
    ~VulkanFrameGraph();

    void AddPass(std::string Name, FrameGraphSubmissionQueue Queue, std::span<const FrameGraphResourceAccess> Reads, std::span<const FrameGraphResourceAccess> Writes, std::function<void(FRHICommandList&)>&& Lambda);
    void AddExternalWait();
    void AddExternalSignal();
    void Compile();
    void Execute(VkSemaphore ImageAcquiredSemaphore, VkSemaphore RenderFinishedSemaphore);

    std::vector<FrameGraphPass> Passes;
    std::vector<FrameGraphPayload> Payloads;

private:
    struct State
    {
        bool FirstUse = true;
        int32_t LastPayloadIndex = -1;   // where it was last written/read
        FrameGraphSubmissionQueue CurrentQueue = FrameGraphSubmissionQueue::Graphics;
    };

    struct FImageState : State
    {
        VkImageLayout CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    };

    struct FBufferState : State
    {
        VkAccessFlags2 AccessMask = 0;
        bool LastAccessWrite = false;
    };

    struct Compiler
    {
        std::unordered_map<uint32_t, FImageState>  ImageStates;
        std::unordered_map<uint32_t, FBufferState> BufferStates;
    };

    Compiler m_Compiler;

    void HandleResourceAccess(const FrameGraphPass& Pass, FrameGraphPayload& Payload, const FrameGraphResourceAccess& ResourceAccess, bool IsWrite);
};
