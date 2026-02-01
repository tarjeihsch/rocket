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
	virtual void Reset() override;
    virtual void Submit(RHICommandList& CmdList) override;
    virtual const char* GetName() const override;
    virtual const char* GetVersion() const override;
    virtual RHIInterfaceType GetInterfaceType() const noexcept override;
    virtual RHI* GetNonValidationRHI() const noexcept override;
    virtual void RHIWaitUntilIdle() const override;
    virtual uint64_t RHIGetFrameCounter() const noexcept override;
    virtual RHISurface* RHICreateSurface(const RHISurfaceDesc& Desc) override;
    virtual RHITexture* RHICreateTexture(const RHITextureDesc& Desc) const override;
    virtual RHITextureView* RHICreateTextureView(const RHITextureViewDesc& TextureViewDesc, RHITexture* Texture) const override;
    virtual RHITextureSampler* RHICreateTextureSampler(const RHITextureSamplerDesc& Desc) const override;
    virtual RHIBuffer* RHICreateBuffer(const RHIBufferDesc& Desc) const override;
    virtual RHIPipeline* RHICreatePipeline(const RHIPipelineDesc& Desc) override;

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
