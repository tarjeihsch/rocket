#pragma once

#include <vma/vk_mem_alloc.h>

#include "Common/RHITexture.h"
#include "VulkanDescriptorManager.h"

class VulkanSurface;
class VulkanDevice;
struct Descriptor;

// Both persistent and transient textures will only ever live within the duration of this wrapper.
struct VulkanTexture : RHITexture
{
private:
    VulkanDevice& device;

public:
    VulkanTexture(VulkanDevice& in_device, RHITextureDesc& in_desc);
    VulkanTexture(VulkanDevice& in_device, RHITextureDesc& in_desc, VkImage in_image_handle);
    ~VulkanTexture();

    virtual void* GetHandle() const override;

    VkImage handle;
    VmaAllocation allocation_handle;
    uint64_t offset;

private:
    void alloc();
    void free();
};

struct VulkanTextureView : RHITextureView
{
private:
    VulkanDevice& Device;

public:
    VulkanTextureView(VulkanDevice& InDevice, RHITextureViewDesc& InDesc, RHITexture* Texture);
    VulkanTextureView(VulkanDevice& InDevice, RHITextureViewDesc& InDesc, RHITexture* Texture, VkImageView InImageView);
    ~VulkanTextureView();

    virtual void* get_handle() const override;
    virtual const RHITextureViewDesc& get_desc() const override;
    virtual uint32_t GetBindingIndex() const override;
    virtual RHITexture* get_target_texture() const override;

    VkImageView image_view_handle;
    RHITexture* target_texture;

private:
    void Alloc();
    void Free();

private:
    RHITextureViewDesc desc;
    Descriptor descriptor_handle;
};
