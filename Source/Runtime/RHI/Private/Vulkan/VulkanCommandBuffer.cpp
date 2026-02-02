#include "VulkanCommandBuffer.h"

#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanQueue.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "Assertion.h"

VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBufferPool& InPool)
    : Pool(InPool),
    State(VulkanCommandBufferState::NotAllocated),
    PoolID(UINT32_MAX)
{
    Alloc();
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    Free();
}

void VulkanCommandBuffer::Alloc()
{
    ensure(State == VulkanCommandBufferState::NotAllocated);

    VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
    CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferAllocateInfo.commandPool = Pool.GetHandle();
    CommandBufferAllocateInfo.commandBufferCount = 1;
    CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkResult Result = VulkanRHI::vkAllocateCommandBuffers(GetVulkanRHIMinimal()->RHIGetVkDevice(), &CommandBufferAllocateInfo, &Handle);
    ensure(Result == VK_SUCCESS, "Failed to allocate command buffers.");

    State = VulkanCommandBufferState::ReadyForBegin;
}

void VulkanCommandBuffer::Free()
{
    ensure(State != VulkanCommandBufferState::NotAllocated);

    VulkanRHI::vkFreeCommandBuffers(GetVulkanRHIMinimal()->RHIGetVkDevice(), Pool.GetHandle(), 1, &Handle);

    State = VulkanCommandBufferState::NotAllocated;
}

void VulkanCommandBuffer::Begin()
{
    ensure(State == VulkanCommandBufferState::ReadyForBegin || State == VulkanCommandBufferState::NeedReset);

    if (State == VulkanCommandBufferState::NeedReset)
    {
        Reset();
    }

    VkCommandBufferBeginInfo CommandBufferBeginInfo = {};
    CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CommandBufferBeginInfo.pNext = nullptr;
    CommandBufferBeginInfo.pInheritanceInfo = nullptr;

    VkResult Result = VulkanRHI::vkBeginCommandBuffer(Handle, &CommandBufferBeginInfo);
    ensure(Result == VK_SUCCESS, "Failed to begin recording command buffer.");

    State = VulkanCommandBufferState::IsInsideBegin;
}

void VulkanCommandBuffer::End()
{
    ensure(State == VulkanCommandBufferState::IsInsideBegin);

    VulkanRHI::vkEndCommandBuffer(Handle);

    State = VulkanCommandBufferState::HasEnded;
}

void VulkanCommandBuffer::Submit()
{
    ensure(State == VulkanCommandBufferState::HasEnded);

    static const auto Start = std::chrono::steady_clock::now();
    const auto Now = std::chrono::steady_clock::now();
    SubmissionTime = std::chrono::duration<double>(Now - Start).count();

    State = VulkanCommandBufferState::Submitted;
}

void VulkanCommandBuffer::Retire()
{
    ensure(State == VulkanCommandBufferState::Submitted);

    // todo: calculate submission time

    State = VulkanCommandBufferState::NeedReset;
}

void VulkanCommandBuffer::Reset()
{
    //ensure(State == EVulkanCommandBufferState::NeedReset);

    VulkanRHI::vkResetCommandBuffer(Handle, 0);

    State = VulkanCommandBufferState::ReadyForBegin;
}

VulkanCommandBufferPool::VulkanCommandBufferPool(VulkanDevice& InDevice, VulkanQueue& InQueue)
    : Device(InDevice),
    Queue(InQueue)
{
    Initialize();
}

VulkanCommandBufferPool::~VulkanCommandBufferPool()
{
    Shutdown();
}

VulkanCommandBuffer* VulkanCommandBufferPool::Acquire()
{
    VulkanCommandBuffer* CommandBuffer = nullptr;

    if (FreeCommandBuffers.empty())
    {
        CommandBuffer = new VulkanCommandBuffer(*this);
    }
    else 
    {
        CommandBuffer = FreeCommandBuffers.front();
        FreeCommandBuffers.front() = FreeCommandBuffers.back();
        FreeCommandBuffers.pop_back();
    }
    CommandBuffer->SetPoolID(UsedCommandBuffers.size());
    UsedCommandBuffers.push_back(CommandBuffer);
    return CommandBuffer;
}

void VulkanCommandBufferPool::Recycle(VulkanCommandBuffer* CommandBuffer)
{
    CommandBuffer->Reset();

    if (CommandBuffer->GetPoolID() != UsedCommandBuffers.size() - 1)
    {
        VulkanCommandBuffer* Temp = UsedCommandBuffers[UsedCommandBuffers.size() - 1];
        UsedCommandBuffers[CommandBuffer->GetPoolID()] = Temp;
        Temp->SetPoolID(CommandBuffer->GetPoolID());
    }

    UsedCommandBuffers.pop_back();
    FreeCommandBuffers.push_back(CommandBuffer);
}

void VulkanCommandBufferPool::Initialize()
{
    VkCommandPoolCreateInfo CommandPoolCreateInfo = {};
    CommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    CommandPoolCreateInfo.pNext = nullptr;
    CommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    CommandPoolCreateInfo.queueFamilyIndex = Queue.GetQueueFamilyIndex();

    VkResult Result = VulkanRHI::vkCreateCommandPool(Device.GetVkDevice(), &CommandPoolCreateInfo, nullptr, &Handle);
    ensure(Result == VK_SUCCESS, "Failed to create command pool.");
}

void VulkanCommandBufferPool::Shutdown()
{
    for (VulkanCommandBuffer* UsedCommandBuffer : UsedCommandBuffers)
    {
        delete UsedCommandBuffer;
    }

    for (VulkanCommandBuffer* FreeCommandBuffer : FreeCommandBuffers)
    {
        delete FreeCommandBuffer;
    }

    VulkanRHI::vkDestroyCommandPool(Device.GetVkDevice(), Handle, nullptr); 
    Handle = nullptr;
}
