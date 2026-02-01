#pragma once

#include <vma/vk_mem_alloc.h>

#include "Common/RHITexture.h"
#include "VulkanDescriptorManager.h"

class VulkanSurface;
class VulkanDevice;
struct RHIDescriptorHandle;

struct VulkanTexture : RHITexture
{
private:
    VulkanDevice& device;

public:
    VulkanTexture(VulkanDevice& in_device, const RHITextureDesc& in_desc);
    VulkanTexture(VulkanDevice& in_device, const RHITextureDesc& in_desc, VkImage in_image_handle);
    ~VulkanTexture();

    virtual void* GetNativeTexture() const override;
    virtual RHITextureView* GetDefaultView() override;

    VkImage NativeHandle;
    VmaAllocation AllocationHandle;
    uint64_t offset;
    struct RHITextureView* default_view = nullptr;

private:
    void alloc();
    void free();
};

struct VulkanTransientTexture
{
};