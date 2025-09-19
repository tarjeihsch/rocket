#pragma once

#include <vector>
#include <cstdint>
#include <vulkan/vulkan_core.h>

struct DescriptorHandle
{
    uint32_t Index = UINT32_MAX;
    uint32_t Generation = UINT32_MAX;

    inline bool IsValid() const
    {
        return Index != UINT32_MAX && Generation != UINT32_MAX;
    }
};

enum class DescriptorBinding : uint32_t
{
    SampledImage = 0,
    Sampler = 1,
    StorageImage = 2,
    StorageBuffer = 3,
    Count
};

struct PushConstantConfig
{
    VkPushConstantRange Range = {};
    uint32_t Size = 0;
    VkShaderStageFlags Stages = 0;
};

class FVulkanDevice;

class VulkanDescriptorManager
{
public:
    explicit VulkanDescriptorManager(FVulkanDevice& p_Device, uint32_t p_MaxSampledImages = 16384, uint32_t p_MaxSamplers = 1024, uint32_t p_MaxStorageImages = 2048, uint32_t p_MaxStorageBuffers = 4096);
    ~VulkanDescriptorManager();

    DescriptorHandle AddSampledImage(VkImageView View, VkImageLayout Layout);
    DescriptorHandle AddSampler(VkSampler Sampler);
    DescriptorHandle AddStorageImage(VkImageView View, VkImageLayout Layout);
    DescriptorHandle AddStorageBuffer(VkBuffer Buffer, VkDeviceSize Size);

    void RemoveSampledImage(DescriptorHandle Handle);
    void RemoveSampler(DescriptorHandle Handle);
    void RemoveStorageImage(DescriptorHandle Handle);
    void RemoveStorageBuffer(DescriptorHandle Handle);

    void Flush();

    inline VkDescriptorSetLayout GetDescriptorSetLayout() const;
    inline VkDescriptorSet GetDescriptorSet() const;
    inline VkDescriptorPool GetDescriptorPool() const;
    inline VkPipelineLayout GetPipelineLayout() const;

private:
    void Init();
    void Shutdown();

private:
    struct SlotBase
    {
        std::vector<uint32_t> Generation;
        std::vector<uint8_t> Dirty;
        std::vector<uint32_t> FreeList;
    };

    struct ImageSlotArray : SlotBase
    {
        std::vector<VkDescriptorImageInfo> Infos;
    };

    struct RSamplerSlotArray : SlotBase
    {
        std::vector<VkDescriptorImageInfo> Infos;
    };

    struct BufferSlotArray : SlotBase
    {
        std::vector<VkDescriptorBufferInfo> Infos;
    };

private:
    DescriptorHandle AllocSlot(SlotBase& Slot);
    void FreeSlot(SlotBase& Slot, DescriptorHandle Handle);
    void MarkDirty(SlotBase& Slot, uint32_t Index);

private:
    FVulkanDevice& Device;

    VkDescriptorSetLayout DescriptorSetLayout;
    VkDescriptorSet DescriptorSet;
    VkDescriptorPool DescriptorPool;
    VkPipelineLayout PipelineLayout;

    ImageSlotArray SampledImages;
    RSamplerSlotArray Samplers;
    ImageSlotArray StorageImages;
    BufferSlotArray StorageBuffers;

    PushConstantConfig m_PushConstantConfig;
};

VkDescriptorSet VulkanDescriptorManager::GetDescriptorSet() const
{
    return DescriptorSet;
}

VkDescriptorSetLayout VulkanDescriptorManager::GetDescriptorSetLayout() const
{
    return DescriptorSetLayout;
}

VkDescriptorPool VulkanDescriptorManager::GetDescriptorPool() const
{
    return DescriptorPool;
}

VkPipelineLayout VulkanDescriptorManager::GetPipelineLayout() const
{
    return PipelineLayout;
}