#pragma once

#include "Vulkan/VulkanRHIMinimal.h"

#include <vector>

struct RHITextureView;
struct FRHITextureAttachment;
class VulkanDevice;
class VulkanSwapchain;
class FrameGraph;

class CVulkanRHI : public IVulkanRHIMinimal
{
public:
    CVulkanRHI();

    // IRHI interface
    virtual void Init() override;
    virtual void Shutdown() override;

    virtual void Submit(RHICommandList& cmd_list) override;
    virtual void Reset() override;

    [[nodiscard]] virtual const char* get_name() const override;
    [[nodiscard]] virtual const char* get_version() const override;
    [[nodiscard]] virtual RHIInterfaceType get_interface_type() const noexcept override;
    [[nodiscard]] virtual RHI* get_non_validation_rhi() const noexcept override;

    virtual void rhi_wait_until_idle() const override;
    virtual uint64_t rhi_get_frame_counter() const noexcept override;
    virtual RHISurface* CreateSurface(RHISurfaceDesc desc) override;
    virtual RHITexture* RHICreateTexture(RHITextureDesc Desc) const override;
    virtual RHITextureView* RHICreateTextureView(RHITextureViewDesc TextureViewDesc, RHITexture* Texture) const override;
    virtual RHIBuffer* RHICreateBuffer(RHIBufferDesc Desc) const override;
    virtual RHISampler* RHICreateSampler(RHISamplerDesc Desc) const override;
    virtual RHIPipeline* RHICreatePipeline(RHIPipelineDesc Desc) override;

    // IVulkanRHIMinimal interface
    virtual VkInstance RHIGetVkInstance() const override;
    virtual VkDevice RHIGetVkDevice() const override;
    virtual VkPhysicalDevice RHIGetVkPhysicalDevice() const override;
    virtual VmaAllocator RHIGetVmaAllocator() const override;
    virtual VkQueue RHIGetGraphicsVkQueue() const override;
    virtual VkQueue RHIGetComputeVkQueue() const override;
    virtual VkQueue RHIGetTransferVkQueue() const override;
    virtual VkQueue RHIGetPresentVkQueue() const override;
    virtual uint32_t RHIGetGraphicsQueueFamilyIndex() const override;
    virtual uint32_t RHIGetComputeQueueFamilyIndex() const override;
    virtual uint32_t RHIGetTransferQueueFamilyIndex() const override;
    virtual uint32_t RHIGetPresentQueueFamilyIndex() const override;
    virtual std::span<const char*> GetValidationExtensions() override;
    virtual std::span<const char*> GetInstanceExtensions() override;
    virtual std::span<const char*> GetPhysicalDeviceExtensions() override;
    // ~IVulkanRHIMinimal interface

    // CVulkanRHI interface
    inline VulkanDevice* GetDevice() const;
    // ~CVulkanRHI interface

private:
    std::vector<const char*> InstanceExtensions;
    std::vector<const char*> ValidationLayerExtensions;
    std::vector<const char*> PhysicalDeviceExtensions;

    VkInstance Instance;
    VulkanDevice* Device;

    uint64_t FrameCounter;
};
