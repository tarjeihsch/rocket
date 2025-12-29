#include "VulkanDevice.h"
#include "Assertion.h"
#include "VulkanPipeline.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanCommandList.h"
#include "VulkanDescriptorManager.h"
#include "VulkanLoader.h"
#include "VulkanQueue.h"
#include "VulkanTexture.h"
#include "VulkanCommandBuffer.h"

#define MB(x) x / 1024 / 1024

void FTransientResourceAllocator::Init()
{
    VkResult Result = VK_SUCCESS;

    VkPhysicalDeviceMemoryProperties MemoryProperties;
    VulkanRHI::vkGetPhysicalDeviceMemoryProperties(Device.GetVkPhysicalDevice(), &MemoryProperties);

    // All memory types are acceptable but regarded unsafe for production. Use vkGetImageMemoryRequirements or vkGetBufferMemoryRequirements instead.
    // TODO: Query correct memory type for proper performance. Different hardware might differ in TypeIndex order.
    uint32_t MemoryTypeBits = 0xFFFFFFFF;
    uint32_t TypeIndex = UINT32_MAX;

    for (uint32_t Index = 0; Index < MemoryProperties.memoryTypeCount; ++Index)
    {
        if ((MemoryTypeBits & (1 << Index)) && ((MemoryProperties.memoryTypes[Index].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)))
        {
            TypeIndex = Index;
            break;
        }
    }

    ensure(TypeIndex != UINT32_MAX);

    VkMemoryAllocateInfo AllocInfo = {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = Size;
    AllocInfo.memoryTypeIndex = TypeIndex;

    Result = VulkanRHI::vkAllocateMemory(Device.GetVkDevice(), &AllocInfo, nullptr, &PhysicalMemory);
    ensure(Result == VK_SUCCESS, "Failed to allocate memory.");

    Result = VulkanRHI::vkMapMemory(Device.GetVkDevice(), PhysicalMemory, 0, Size, 0, &MappedBasePtr);
    ensure(Result == VK_SUCCESS, "Failed to map transient memory.");

    VmaVirtualBlockCreateInfo BlockCreateInfo = {};
    BlockCreateInfo.size = 64ull * 1024 * 1024;
    BlockCreateInfo.flags = 0;

    Result = vmaCreateVirtualBlock(&BlockCreateInfo, &VirtualMemory);
    ensure(Result == VK_SUCCESS, "Failed to create virtual block.");

    RK_LOG_INFO("Reserved virtual block for transient resources: {} MB", MB(Size));
}

void FTransientResourceAllocator::Shutdown()
{
    VulkanRHI::vkFreeMemory(GetVulkanRHIMinimal()->RHIGetVkDevice(), PhysicalMemory, nullptr);
    vmaClearVirtualBlock(VirtualMemory);
    vmaDestroyVirtualBlock(VirtualMemory);
}

void FTransientResourceAllocator::Reset()
{
    vmaClearVirtualBlock(VirtualMemory);
    Offset = 0;
}

void FTransientResourceAllocator::AllocImage(VkImage Image, uint64_t& OutOffset)
{
    VkMemoryRequirements MemoryRequirements;
    VulkanRHI::vkGetImageMemoryRequirements(Device.GetVkDevice(), Image, &MemoryRequirements);

    VmaVirtualAllocationCreateInfo AllocInfo = {};
    AllocInfo.size = MemoryRequirements.size;
    AllocInfo.alignment = MemoryRequirements.alignment;

    // We can safely throw away the allocation and offset info given by vmaVirtualAllocate, as we are resetting the entire allocator each frame.
    VmaVirtualAllocation Allocation;

    VkResult Result = vmaVirtualAllocate(VirtualMemory, &AllocInfo, &Allocation, &OutOffset);
    ensure(Result == VK_SUCCESS, "Failed to allocate transient image memory.");
    VulkanRHI::vkBindImageMemory(Device.GetVkDevice(), Image, PhysicalMemory, OutOffset);

    CurrentAllocations++;
    TotalAllocations++;
    Offset += MemoryRequirements.size;
}

void FTransientResourceAllocator::AllocBuffer(VkBuffer Buffer, uint64_t& OutOffset)
{
    VkMemoryRequirements MemoryRequirements;
    VulkanRHI::vkGetBufferMemoryRequirements(Device.GetVkDevice(), Buffer, &MemoryRequirements);

    VmaVirtualAllocationCreateInfo AllocInfo = {};
    AllocInfo.size = MemoryRequirements.size;
    AllocInfo.alignment = MemoryRequirements.alignment;

    // We can safely throw away the allocation and offset info given by vmaVirtualAllocate, as we are resetting the entire allocator each frame.
    VmaVirtualAllocation Allocation;

    VkResult Result = vmaVirtualAllocate(VirtualMemory, &AllocInfo, &Allocation, &OutOffset);
    ensure(Result == VK_SUCCESS, "Failed to allocate transient buffer memory.");
    VulkanRHI::vkBindBufferMemory(Device.GetVkDevice(), Buffer, PhysicalMemory, OutOffset);

    CurrentAllocations++;
    TotalAllocations++;
    Offset += MemoryRequirements.size;
}

void *FTransientResourceAllocator::GetMappedBasePtr() const
{
    return MappedBasePtr;
}

FDeferredDeletionQueue::FDeferredDeletionQueue(FVulkanDevice &InDevice) : Device(InDevice)
{
}

FDeferredDeletionQueue::~FDeferredDeletionQueue()
{
    ReleaseExpiredResources(UINT64_MAX);
}

void FDeferredDeletionQueue::EnqueueGenericResource(EType Type, uint64_t Handle, RDescriptorHandle BindingIndex, VmaAllocation Allocation)
{
    FEntry Entry;
    Entry.StructureType = Type;
    Entry.Handle = Handle;
    Entry.Allocation = Allocation;
    Entry.FrameNumber = GetRHI()->GetFrameCounter();
    Entry.m_DescriptorHandle = BindingIndex;

    Entries.push_back(Entry);
}

void FDeferredDeletionQueue::ReleaseExpiredResources(uint64_t CompletedFrameNumber)
{
    for (auto It = Entries.begin(); It != Entries.end(); )
    {
        if (It->FrameNumber > CompletedFrameNumber)
        {
            // Further defer destruction of objects that are in use
            It++;
            continue;
        }

        switch (It->StructureType)
        {
        case EType::Image: VulkanRHI::vkDestroyImage(Device.GetVkDevice(), (VkImage)It->Handle, nullptr); break;
        case EType::ImageView:
            VulkanRHI::vkDestroyImageView(Device.GetVkDevice(), (VkImageView)It->Handle, nullptr);
            if (It->m_DescriptorHandle.IsValid()) { Device.GetDescriptorManager()->RemoveSampledImage(It->m_DescriptorHandle); }
            break;
        case EType::Buffer:
            VulkanRHI::vkDestroyBuffer(Device.GetVkDevice(), (VkBuffer)It->Handle, nullptr);
            if (It->m_DescriptorHandle.IsValid()) { Device.GetDescriptorManager()->RemoveStorageBuffer(It->m_DescriptorHandle); }
            break;
        case EType::Sampler:
            VulkanRHI::vkDestroySampler(Device.GetVkDevice(), (VkSampler)It->Handle, nullptr);
            if (It->m_DescriptorHandle.IsValid()) { Device.GetDescriptorManager()->RemoveSampler(It->m_DescriptorHandle); }
            break;
        default: break;
        }

        if (It->Allocation != nullptr)
        {
            vmaFreeMemory(Device.GetVmaAllocator(), It->Allocation);
        }

        It = Entries.erase(It);
    }
}

FVulkanDevice::FVulkanDevice(VkPhysicalDevice InPhysicalDevice)
    : PhysicalDevice(InPhysicalDevice)
{
    Initialize();
}

FVulkanDevice::~FVulkanDevice()
{
    Shutdown();
}

void FVulkanDevice::WaitUntilIdle() const
{
    VulkanRHI::vkDeviceWaitIdle(Device);
}

VulkanCommandListContext *FVulkanDevice::GetImmediateCommandListContext() const
{
    return ImmediateCommandListContext;
}

void FVulkanDevice::GC()
{
    TransientResourceAllocator->Reset();

    DeferredDeletionQueue->ReleaseExpiredResources(GetRHI()->GetFrameCounter());
}

void FVulkanDevice::Initialize()
{
    uint32_t QueueFamilyCount = 0;
    VulkanRHI::vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> QueueFamiliesProperties(QueueFamilyCount);
    VulkanRHI::vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueFamilyCount, QueueFamiliesProperties.data());

    std::vector<float> QueuePriorities(QueueFamilyCount, 1.0f);
    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos(QueueFamilyCount);
    for (uint32_t Index = 0; Index < QueueFamilyCount; Index++)
    {
	    VkDeviceQueueCreateInfo QueueFamilyCreateInfo = {};
	    QueueFamilyCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	    QueueFamilyCreateInfo.queueCount = 1;
	    QueueFamilyCreateInfo.queueFamilyIndex = Index;
	    QueueFamilyCreateInfo.pQueuePriorities = &QueuePriorities[Index];
	    QueueCreateInfos[Index] = QueueFamilyCreateInfo;
    }

    // Vulkan 1.3 features
    VkPhysicalDeviceVulkan13Features Features_1_3 = {};
    Features_1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    Features_1_3.dynamicRendering = VK_TRUE;
    Features_1_3.synchronization2 = VK_TRUE;

    // Vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features Features_1_2 = {};
    Features_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    Features_1_2.bufferDeviceAddress = VK_TRUE;
    Features_1_2.bufferDeviceAddressCaptureReplay = VK_TRUE;
    Features_1_2.descriptorIndexing = VK_TRUE;
    Features_1_2.runtimeDescriptorArray = VK_TRUE;
    Features_1_2.descriptorBindingPartiallyBound = VK_TRUE;
    Features_1_2.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    Features_1_2.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
    Features_1_2.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    Features_1_2.timelineSemaphore = VK_TRUE;
    Features_1_2.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;

    // Chain the features together
    Features_1_3.pNext = &Features_1_2;

    VkPhysicalDeviceFeatures DeviceFeatures = {};
    DeviceFeatures.shaderInt64 = VK_TRUE;
    DeviceFeatures.samplerAnisotropy = VK_TRUE;
    DeviceFeatures.multiDrawIndirect = VK_TRUE;

    VkDeviceCreateInfo DeviceCreateInfo = {};
    DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
    DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
    DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;
    DeviceCreateInfo.pNext = &Features_1_3;
    DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(GetVulkanRHIMinimal()->GetPhysicalDeviceExtensions().size());
    DeviceCreateInfo.ppEnabledExtensionNames = GetVulkanRHIMinimal()->GetPhysicalDeviceExtensions().data();
    DeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(GetVulkanRHIMinimal()->GetValidationExtensions().size());
    DeviceCreateInfo.ppEnabledLayerNames = GetVulkanRHIMinimal()->GetValidationExtensions().data();

    VkResult Result = VulkanRHI::vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, nullptr, &Device);
    ensure(Result == VK_SUCCESS, "Failed to initialize logical device.");

    LoadVulkanDeviceFunc(GetVkDevice());

    VmaVulkanFunctions VmaFuncs = CreateVmaFuncTable();
    VmaAllocatorCreateInfo AllocatorCreateInfo = {};
    AllocatorCreateInfo.physicalDevice = PhysicalDevice;
    AllocatorCreateInfo.device = Device;
    AllocatorCreateInfo.instance = GetVulkanRHIMinimal()->RHIGetVkInstance();
    AllocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    AllocatorCreateInfo.pVulkanFunctions = &VmaFuncs;

    Result = vmaCreateAllocator(&AllocatorCreateInfo, &Allocator);
    ensure(Result == VK_SUCCESS, "Failed to initialize memory allocator.");

    for (uint32_t Index = 0; Index < QueueFamilyCount; Index++)
    {
        if ((QueueFamiliesProperties[Index].queueFlags & ~(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT)) != 0)
        {
            continue;
        }

        Queues.push_back(new FVulkanQueue(*this, QueueFamiliesProperties[Index].queueFlags, false, Index, 1));
    }

    PipelineManager = new FVulkanPipelineManager(*this);
    DescriptorManager = new VulkanDescriptorManager(*this);
    DeferredDeletionQueue = new FDeferredDeletionQueue(*this);
    CommandListContext = new VulkanCommandListContext(*this);
    TransientResourceAllocator = new FTransientResourceAllocator(*this, 32ull * 1024 * 1024);
}

void FVulkanDevice::Shutdown()
{
    delete PipelineManager;

    delete DescriptorManager;

    delete DeferredDeletionQueue;

    delete CommandListContext;

    delete TransientResourceAllocator;

    for (auto& Queue : Queues)
    {
        delete Queue;
    }

    vmaDestroyAllocator(Allocator);
    VulkanRHI::vkDestroyDevice(Device, nullptr);
}

FVulkanQueue* FVulkanDevice::GetQueue(VkQueueFlags Include, VkQueueFlags Exclude) const
{
    for (const auto& Queue : Queues)
    {
        if ((Queue->GetQueueFlags() & Include) == Include && (Queue->GetQueueFlags() & Exclude) == 0)
        {
            return Queue;
        }
    }
    return nullptr;
}

FVulkanQueue* FVulkanDevice::GetGraphicsQueue() const
{
    return GetQueue(VK_QUEUE_GRAPHICS_BIT, 0);
}

FVulkanQueue* FVulkanDevice::GetComputeQueue() const
{
    return GetQueue(VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT);
}

FVulkanQueue* FVulkanDevice::GetTransferQueue() const
{
    return GetQueue(VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT);
}

FVulkanQueue* FVulkanDevice::GetPresentQueue() const
{
    return GetQueue(VK_QUEUE_GRAPHICS_BIT, 0);
}
