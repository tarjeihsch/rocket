#include "VulkanDescriptorManager.h"

#include "VulkanLoader.h"
#include "VulkanDevice.h"
#include "Assertion.h"

static VkDescriptorSetLayoutBinding MakeBinding(uint32_t pBinding, VkDescriptorType pType, VkShaderStageFlags pStages, uint32_t pCount)
{
    VkDescriptorSetLayoutBinding Binding = {};
    Binding.binding = pBinding;
    Binding.descriptorType = pType;
    Binding.descriptorCount = pCount;
    Binding.stageFlags = pStages;
    return Binding;
}

VulkanDescriptorManager::VulkanDescriptorManager(VulkanDevice& pDevice, uint32_t MaxSampledImages, uint32_t MaxSamplers, uint32_t MaxStorageImages, uint32_t MaxStorageBuffers)
    : Device(pDevice), DescriptorSetLayout(VK_NULL_HANDLE), DescriptorSet(VK_NULL_HANDLE), DescriptorPool(VK_NULL_HANDLE)
{
    SampledImages.Infos.resize(MaxSampledImages);
    SampledImages.Generation.resize(MaxSampledImages, 1);
    SampledImages.Dirty.resize(MaxSampledImages);

    Samplers.Infos.resize(MaxSamplers);
    Samplers.Generation.resize(MaxSamplers, 1);
    Samplers.Dirty.resize(MaxSamplers);

    StorageImages.Infos.resize(MaxStorageImages);
    StorageImages.Generation.resize(MaxStorageImages, 1);
    StorageImages.Dirty.resize(MaxStorageImages);

    StorageBuffers.Infos.resize(MaxStorageBuffers);
    StorageBuffers.Generation.resize(MaxStorageBuffers, 1);
    StorageBuffers.Dirty.resize(MaxStorageBuffers);

    for (uint32_t Index = 0; Index < MaxSampledImages; ++Index) 
    {
        SampledImages.FreeList.push_back(MaxSampledImages - 1 - Index);
    }

    for (uint32_t Index = 0; Index < MaxSamplers; ++Index) 
    {
        Samplers.FreeList.push_back(MaxSamplers - 1 - Index);
    }

    for (uint32_t Index = 0; Index < MaxStorageImages; ++Index) 
    {
        StorageImages.FreeList.push_back(MaxStorageImages - 1 - Index);
    }

    for (uint32_t Index = 0; Index < MaxStorageBuffers; ++Index) 
    {
        StorageBuffers.FreeList.push_back(MaxStorageBuffers - 1 - Index);
    }

    VkDescriptorSetLayoutBinding Bindings[static_cast<uint32_t>(DescriptorBinding::Count)] = {};
    Bindings[static_cast<uint32_t>(DescriptorBinding::SampledImage)]    = MakeBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_ALL, MaxSampledImages);
    Bindings[static_cast<uint32_t>(DescriptorBinding::Sampler)]         = MakeBinding(1, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_ALL, MaxSamplers);
    Bindings[static_cast<uint32_t>(DescriptorBinding::StorageImage)]    = MakeBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_ALL, MaxStorageImages);
    Bindings[static_cast<uint32_t>(DescriptorBinding::StorageBuffer)]   = MakeBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_ALL, MaxStorageBuffers);

    std::vector<VkDescriptorBindingFlags> Flags(static_cast<uint32_t>(DescriptorBinding::Count));
    Flags[static_cast<uint32_t>(DescriptorBinding::SampledImage)]       = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;
    Flags[static_cast<uint32_t>(DescriptorBinding::Sampler)]            = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
    Flags[static_cast<uint32_t>(DescriptorBinding::StorageImage)]       = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
    Flags[static_cast<uint32_t>(DescriptorBinding::StorageBuffer)]      = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

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

    VkResult Result = VulkanRHI::vkCreateDescriptorSetLayout(pDevice.get_vk_device(), &LayoutCreateInfo, nullptr, &DescriptorSetLayout);
    ensure(Result == VK_SUCCESS);

    VkDescriptorPoolSize PoolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,  MaxSampledImages },
        { VK_DESCRIPTOR_TYPE_SAMPLER,        MaxSamplers },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  MaxStorageImages },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MaxStorageBuffers },
    };

    VkDescriptorPoolCreateInfo PoolCreateInfo = {};
    PoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolCreateInfo.maxSets = 1;
    PoolCreateInfo.poolSizeCount = static_cast<uint32_t>(DescriptorBinding::Count);
    PoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    PoolCreateInfo.pPoolSizes = PoolSizes;
    Result = VulkanRHI::vkCreateDescriptorPool(pDevice.get_vk_device(), &PoolCreateInfo, nullptr, &DescriptorPool);
    ensure(Result == VK_SUCCESS);

    VkDescriptorSetAllocateInfo DescriptorSetAllocInfo = {};
    DescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    DescriptorSetAllocInfo.descriptorPool     = DescriptorPool;
    DescriptorSetAllocInfo.descriptorSetCount = 1;
    DescriptorSetAllocInfo.pSetLayouts        = &DescriptorSetLayout;
    Result = VulkanRHI::vkAllocateDescriptorSets(pDevice.get_vk_device(), &DescriptorSetAllocInfo, &DescriptorSet);
    ensure(Result == VK_SUCCESS);

    VkPushConstantRange PushConstantRanges[1];
    PushConstantRanges[0].stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    PushConstantRanges[0].offset = 0;
    PushConstantRanges[0].size = 128; // Target minimum push constant capacity

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {};
    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutCreateInfo.setLayoutCount = 1;
    PipelineLayoutCreateInfo.pSetLayouts = &DescriptorSetLayout;
    PipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    PipelineLayoutCreateInfo.pPushConstantRanges = PushConstantRanges;
    VulkanRHI::vkCreatePipelineLayout(Device.get_vk_device(), &PipelineLayoutCreateInfo, nullptr, &PipelineLayout);
    ensure(Result == VK_SUCCESS);
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
    VulkanRHI::vkDestroyDescriptorPool(Device.get_vk_device(), DescriptorPool, nullptr);
    VulkanRHI::vkDestroyDescriptorSetLayout(Device.get_vk_device(), DescriptorSetLayout, nullptr);
    VulkanRHI::vkDestroyPipelineLayout(Device.get_vk_device(), PipelineLayout, nullptr);
}

Descriptor VulkanDescriptorManager::AllocSlot(VulkanDescriptorManager::SlotBase &Slot)
{
    ensure(!Slot.FreeList.empty(), "Out of slots.");
    uint32_t Index = Slot.FreeList.back();
    Slot.FreeList.pop_back();
    return { Index, Slot.Generation[Index] };
}

void VulkanDescriptorManager::FreeSlot(VulkanDescriptorManager::SlotBase &Slot, Descriptor Handle)
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

Descriptor VulkanDescriptorManager::add_sampled_image(VkImageView View, VkImageLayout Layout)
{
    Descriptor Handle = AllocSlot(SampledImages);
    SampledImages.Infos[Handle.Index] = { VK_NULL_HANDLE, View, Layout };
    MarkDirty(SampledImages, Handle.Index);
    return Handle;
}

Descriptor VulkanDescriptorManager::AddSampler(VkSampler Sampler)
{
    Descriptor Handle = AllocSlot(Samplers);
    Samplers.Infos[Handle.Index] = { Sampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED };
    MarkDirty(Samplers, Handle.Index);
    return Handle;
}

Descriptor VulkanDescriptorManager::add_storage_image(VkImageView View, VkImageLayout Layout)
{
    Descriptor Handle = AllocSlot(StorageImages);
    StorageImages.Infos[Handle.Index] = { VK_NULL_HANDLE, View, Layout };
    MarkDirty(StorageImages, Handle.Index);
    return Handle;
}

Descriptor VulkanDescriptorManager::add_storage_buffer(VkBuffer Buffer, VkDeviceSize Size)
{
    Descriptor Handle = AllocSlot(StorageBuffers);
    StorageBuffers.Infos[Handle.Index] = { Buffer, 0, Size };
    MarkDirty(StorageBuffers, Handle.Index);
    return Handle;
}

void VulkanDescriptorManager::RemoveSampledImage(Descriptor Handle)
{
    // TODO: MarkDirty? Write a dummy null descriptor?
    FreeSlot(SampledImages, Handle);
}

void VulkanDescriptorManager::RemoveSampler(Descriptor Handle)
{
    // TODO: MarkDirty? Write a dummy null descriptor?
    FreeSlot(Samplers, Handle);
}

void VulkanDescriptorManager::RemoveStorageImage(Descriptor Handle)
{
    // TODO: MarkDirty? Write a dummy null descriptor?
    FreeSlot(StorageImages, Handle);
}

void VulkanDescriptorManager::RemoveStorageBuffer(Descriptor Handle)
{
    // TODO: MarkDirty? Write a dummy null descriptor?
    FreeSlot(StorageBuffers, Handle);
}

void VulkanDescriptorManager::Update()
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

    auto FlushDirtySamplers = [&](DescriptorBinding Binding, RSamplerSlotArray& Slots)
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

    VulkanRHI::vkUpdateDescriptorSets(Device.get_vk_device(), static_cast<uint32_t>(Writes.size()), Writes.data(), 0, nullptr);
}
