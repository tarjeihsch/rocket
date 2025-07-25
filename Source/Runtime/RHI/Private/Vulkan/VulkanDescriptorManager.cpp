#include "VulkanDescriptorManager.h"

#include "VulkanLoader.h"
#include "VulkanDevice.h"
#include "Assert.h"

static VkDescriptorSetLayoutBinding MakeBinding(uint32_t pBinding, VkDescriptorType pType, VkShaderStageFlags pStages, uint32_t pCount)
{
    VkDescriptorSetLayoutBinding Binding = {};
    Binding.binding = pBinding;
    Binding.descriptorType = pType;
    Binding.descriptorCount = pCount;
    Binding.stageFlags = pStages;
    return Binding;
}

VulkanDescriptorManager::VulkanDescriptorManager(FVulkanDevice& pDevice, uint32_t pMaxSampledImages, uint32_t pMaxSamplers, uint32_t pMaxStorageImages, uint32_t pMaxStorageBuffers)
    : Device(pDevice), DescriptorSetLayout(VK_NULL_HANDLE), DescriptorSet(VK_NULL_HANDLE), DescriptorPool(VK_NULL_HANDLE)
{
    SampledImages.Infos.resize(pMaxSampledImages);
    SampledImages.Generation.resize(pMaxSampledImages, 1);
    SampledImages.Dirty.resize(pMaxSampledImages);

    Samplers.Infos.resize(pMaxSamplers);
    Samplers.Generation.resize(pMaxSamplers, 1);
    Samplers.Dirty.resize(pMaxSamplers);

    StorageImages.Infos.resize(pMaxStorageImages);
    StorageImages.Generation.resize(pMaxStorageImages, 1);
    StorageImages.Dirty.resize(pMaxStorageImages);

    StorageBuffers.Infos.resize(pMaxStorageBuffers);
    StorageBuffers.Generation.resize(pMaxStorageBuffers, 1);
    StorageBuffers.Dirty.resize(pMaxStorageBuffers);

    for (uint32_t Index = 0; Index < pMaxSampledImages; ++Index) SampledImages.FreeList.push_back(pMaxSampledImages - 1 - Index);
    for (uint32_t Index = 0; Index < pMaxSamplers; ++Index) Samplers.FreeList.push_back(pMaxSamplers - 1 - Index);
    for (uint32_t Index = 0; Index < pMaxStorageImages; ++Index) StorageImages.FreeList.push_back(pMaxStorageImages - 1 - Index);
    for (uint32_t Index = 0; Index < pMaxStorageBuffers; ++Index) StorageBuffers.FreeList.push_back(pMaxStorageBuffers - 1 - Index);

    VkDescriptorSetLayoutBinding Bindings[static_cast<uint32_t>(DescriptorBinding::Count)] = {};
    Bindings[static_cast<uint32_t>(DescriptorBinding::SampledImage)] = MakeBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL, pMaxSampledImages);
    Bindings[static_cast<uint32_t>(DescriptorBinding::Sampler)] = MakeBinding(1, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL, pMaxSamplers);
    Bindings[static_cast<uint32_t>(DescriptorBinding::StorageImage)] = MakeBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL, pMaxStorageImages);
    Bindings[static_cast<uint32_t>(DescriptorBinding::StorageBuffer)] = MakeBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL, pMaxStorageBuffers);

    std::vector<VkDescriptorBindingFlags> Flags(static_cast<uint32_t>(DescriptorBinding::Count));
    Flags[static_cast<uint32_t>(DescriptorBinding::SampledImage)] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;
    Flags[static_cast<uint32_t>(DescriptorBinding::Sampler)] = 0;
    Flags[static_cast<uint32_t>(DescriptorBinding::StorageImage)] = 0;
    Flags[static_cast<uint32_t>(DescriptorBinding::StorageBuffer)] = 0;

    VkDescriptorSetLayoutBindingFlagsCreateInfo Extension = {};
    Extension.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    Extension.bindingCount = static_cast<uint32_t>(DescriptorBinding::Count);
    Extension.pBindingFlags = Flags.data();

    VkDescriptorSetLayoutCreateInfo LayoutCreateInfo = {};
    LayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutCreateInfo.bindingCount = static_cast<uint32_t>(DescriptorBinding::Count);
    LayoutCreateInfo.pBindings = Bindings;
    LayoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    LayoutCreateInfo.pNext = &Extension;

    VkResult Result = VulkanRHI::vkCreateDescriptorSetLayout(pDevice.GetVkDevice(), &LayoutCreateInfo, nullptr, &DescriptorSetLayout);
    RK_ASSERT(Result == VK_SUCCESS);

    VkDescriptorPoolSize PoolSizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  pMaxSampledImages },
            { VK_DESCRIPTOR_TYPE_SAMPLER,        pMaxSamplers },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  pMaxStorageImages },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, pMaxStorageBuffers },
    };

    VkDescriptorPoolCreateInfo PoolCreateInfo = {};
    PoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolCreateInfo.maxSets = 1;
    PoolCreateInfo.poolSizeCount = static_cast<uint32_t>(DescriptorBinding::Count);
    PoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    PoolCreateInfo.pPoolSizes = PoolSizes;
    Result = VulkanRHI::vkCreateDescriptorPool(pDevice.GetVkDevice(), &PoolCreateInfo, nullptr, &DescriptorPool);
    RK_ASSERT(Result == VK_SUCCESS);

    VkDescriptorSetAllocateInfo DescriptorSetAllocInfo = {};
    DescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    DescriptorSetAllocInfo.descriptorPool     = DescriptorPool;
    DescriptorSetAllocInfo.descriptorSetCount = 1;
    DescriptorSetAllocInfo.pSetLayouts        = &DescriptorSetLayout;
    Result = VulkanRHI::vkAllocateDescriptorSets(pDevice.GetVkDevice(), &DescriptorSetAllocInfo, &DescriptorSet);
    RK_ASSERT(Result == VK_SUCCESS);

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {};
    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutCreateInfo.setLayoutCount = 1;
    PipelineLayoutCreateInfo.pSetLayouts = &DescriptorSetLayout;
    VulkanRHI::vkCreatePipelineLayout(Device.GetVkDevice(), &PipelineLayoutCreateInfo, nullptr, &PipelineLayout);
    RK_ASSERT(Result == VK_SUCCESS);
}

VulkanDescriptorManager::~VulkanDescriptorManager()
{
    Shutdown();
}

void VulkanDescriptorManager::Init()
{

}

void VulkanDescriptorManager::Shutdown()
{
    VulkanRHI::vkDestroyDescriptorPool(Device.GetVkDevice(), DescriptorPool, nullptr);
    VulkanRHI::vkDestroyDescriptorSetLayout(Device.GetVkDevice(), DescriptorSetLayout, nullptr);
    VulkanRHI::vkDestroyPipelineLayout(Device.GetVkDevice(), PipelineLayout, nullptr);
}

DescriptorHandle VulkanDescriptorManager::AllocSlot(VulkanDescriptorManager::SlotBase &Slot)
{
    RK_ASSERT(!Slot.FreeList.empty(), "Out of slots.");
    uint32_t Index = Slot.FreeList.back();
    Slot.FreeList.pop_back();
    return { Index, Slot.Generation[Index] };
}

void VulkanDescriptorManager::FreeSlot(VulkanDescriptorManager::SlotBase &Slot, DescriptorHandle Handle)
{
    if (!Handle.IsValid())
    {
        return;
    }

    if (Slot.Generation[Handle.Index] != Handle.Generation)
    {
        return;
    }

    ++Slot.Generation[Handle.Index];
    Slot.FreeList.push_back(Handle.Index);
}

void VulkanDescriptorManager::MarkDirty(VulkanDescriptorManager::SlotBase &Slot, uint32_t Index)
{
    Slot.Dirty[Index] = 1;
}

DescriptorHandle VulkanDescriptorManager::AddSampledImage(VkImageView View, VkImageLayout Layout)
{
    DescriptorHandle Handle = AllocSlot(SampledImages);
    SampledImages.Infos[Handle.Index] = { VK_NULL_HANDLE, View, Layout };
    MarkDirty(SampledImages, Handle.Index);
    return Handle;
}

DescriptorHandle VulkanDescriptorManager::AddSampler(VkSampler Sampler)
{
    DescriptorHandle Handle = AllocSlot(Samplers);
    SampledImages.Infos[Handle.Index] = { Sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED };
    MarkDirty(Samplers, Handle.Index);
    return Handle;
}

DescriptorHandle VulkanDescriptorManager::AddStorageImage(VkImageView View, VkImageLayout Layout)
{
    DescriptorHandle Handle = AllocSlot(StorageImages);
    StorageImages.Infos[Handle.Index] = { VK_NULL_HANDLE, View, Layout };
    MarkDirty(StorageImages, Handle.Index);
    return Handle;
}

DescriptorHandle VulkanDescriptorManager::AddStorageBuffer(VkBuffer Buffer, VkDeviceSize Size)
{
    DescriptorHandle Handle = AllocSlot(StorageBuffers);
    StorageBuffers.Infos[Handle.Index] = { Buffer, 0, Size };
    MarkDirty(StorageBuffers, Handle.Index);
    return Handle;
}

void VulkanDescriptorManager::RemoveSampledImage(DescriptorHandle Handle)
{
    // TODO: MarkDirty? Write a dummy null descriptor?
    FreeSlot(SampledImages, Handle);
}

void VulkanDescriptorManager::RemoveSampler(DescriptorHandle Handle)
{
    // TODO: MarkDirty? Write a dummy null descriptor?
    FreeSlot(Samplers, Handle);
}

void VulkanDescriptorManager::RemoveStorageImage(DescriptorHandle Handle)
{
    // TODO: MarkDirty? Write a dummy null descriptor?
    FreeSlot(StorageImages, Handle);
}

void VulkanDescriptorManager::RemoveStorageBuffer(DescriptorHandle Handle)
{
    // TODO: MarkDirty? Write a dummy null descriptor?
    FreeSlot(StorageBuffers, Handle);
}

void VulkanDescriptorManager::Flush()
{
    std::vector<VkWriteDescriptorSet> Writes;
    Writes.reserve(1024);

    auto FlushDirtySampledImages = [&](DescriptorBinding Binding, ImageSlotArray& Slots)
    {
        const auto BindingValue = static_cast<uint32_t>(Binding);
        for (uint32_t Index = 0; Index < Slots.Dirty.size(); ++Index)
        {
            if (!Slots.Dirty[Index])
            {
                continue;
            }

            Slots.Dirty[Index] = 0;

            VkWriteDescriptorSet Write = {};
            Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            Write.dstSet = DescriptorSet;
            Write.dstBinding = BindingValue;
            Write.dstArrayElement = Index;
            Write.descriptorCount = 1;
            Write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            Write.pImageInfo = &Slots.Infos[Index];
            Writes.push_back(Write);
        }
    };

    auto FlushDirtySamplers = [&](DescriptorBinding Binding, ImageSlotArray& Slots)
    {
        const auto BindingValue = static_cast<uint32_t>(Binding);
        for (uint32_t Index = 0; Index < Slots.Dirty.size(); ++Index)
        {
            if (!Slots.Dirty[Index])
            {
                continue;
            }

            Slots.Dirty[Index] = 0;

            VkWriteDescriptorSet Write = {};
            Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            Write.dstSet = DescriptorSet;
            Write.dstBinding = BindingValue;
            Write.dstArrayElement = Index;
            Write.descriptorCount = 1;
            Write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            Write.pImageInfo = &Slots.Infos[Index];
            Writes.push_back(Write);
        }
    };

    auto FlushDirtyStorageImages = [&](DescriptorBinding Binding, ImageSlotArray& Slots)
    {
        const auto BindingValue = static_cast<uint32_t>(Binding);
        for (uint32_t Index = 0; Index < Slots.Dirty.size(); ++Index)
        {
            if (!Slots.Dirty[Index])
            {
                continue;
            }

            Slots.Dirty[Index] = 0;

            VkWriteDescriptorSet Write = {};
            Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            Write.dstSet = DescriptorSet;
            Write.dstBinding = BindingValue;
            Write.dstArrayElement = Index;
            Write.descriptorCount = 1;
            Write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            Write.pImageInfo = &Slots.Infos[Index];
            Writes.push_back(Write);
        }
    };

    auto FlushDirtyStorageBuffers = [&](DescriptorBinding Binding, BufferSlotArray& Slots)
    {
        const auto BindingValue = static_cast<uint32_t>(Binding);
        for (uint32_t Index = 0; Index < Slots.Dirty.size(); ++Index)
        {
            if (!Slots.Dirty[Index])
            {
                continue;
            }

            Slots.Dirty[Index] = 0;

            VkWriteDescriptorSet Write = {};
            Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            Write.dstSet = DescriptorSet;
            Write.dstBinding = BindingValue;
            Write.dstArrayElement = Index;
            Write.descriptorCount = 1;
            Write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            Write.pBufferInfo = &Slots.Infos[Index];
            Writes.push_back(Write);
        }
    };

    FlushDirtySampledImages(DescriptorBinding::SampledImage, SampledImages);
    FlushDirtySamplers(DescriptorBinding::Sampler, Samplers);
    FlushDirtyStorageImages(DescriptorBinding::StorageImage, StorageImages);
    FlushDirtyStorageBuffers(DescriptorBinding::StorageBuffer, StorageBuffers);

    VulkanRHI::vkUpdateDescriptorSets(Device.GetVkDevice(), static_cast<uint32_t>(Writes.size()), Writes.data(), 0, nullptr);
}