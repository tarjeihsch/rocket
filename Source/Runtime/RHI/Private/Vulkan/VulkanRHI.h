#pragma once

#include "Vulkan/VulkanRHIMinimal.h"

#include <vector>

struct FRHITextureView;
struct FRHITextureAttachment;
class FVulkanDevice;
class RVulkanSwapchain;
class FFrameGraph;

class CVulkanRHI : public IVulkanRHIMinimal
{
public:
    // IRHI interface
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void Tick(float DeltaTime) override;

    [[nodiscard]] virtual const char* GetName() const override;
    [[nodiscard]] virtual const char* GetVersion() const override;
    [[nodiscard]] virtual ERHIInterfaceType GetInterfaceType() const noexcept override;
    [[nodiscard]] virtual IRHI* GetNonValidationRHI() const noexcept override;

    virtual RRHISwapchain* GetSwapchain() const override;
    virtual void RHIWaitUntilIdle() const override;
    virtual uint64_t GetFrameCounter() const noexcept override;
    virtual FRHITexture* RHICreateTexture(FRHITextureDesc Desc) const override;
    virtual FRHITextureView* RHICreateTextureView(FRHITextureViewDesc TextureViewDesc, FRHITexture* Texture) const override;
    virtual FRHIBuffer* RHICreateBuffer(FRHIBufferDesc Desc) const override;
    virtual RRHISampler* RHICreateSampler(RRHISamplerDesc Desc) const override;
    virtual void RHIUploadDataBlocking(FRHIBuffer* Buffer, void* Data, uint64_t Size) override;
    virtual void RHIUploadTextureBlocking(FRHITexture* Texture2D, const uint8_t* Data, uint32_t Width, uint32_t Height) override;
    virtual FRHIPipeline* RHICreatePipelineStateObject(FRHIPipelineDesc Desc) override;
    virtual IRHICommandListContext* GetCommandListContext() const override;
    virtual void RHICmdBindPipeline(FRHICommandList& RHICmdList, FRHIPipeline* Pipeline) override;
    virtual void RHICmdBeginRendering(FRHICommandList* RHICmdList, std::span<FRHITextureView*> ColorAttachments, FRHITextureView* DepthAttachmentView, bool bClearColor) override;
    virtual void RHICmdEndRendering(FRHICommandList* RHICmdList) override;
    virtual void RHICmdPipelineBarrier(FRHICommandList* RHICmdList, FRHITexture* Texture, ERHITextureLayout SrcLayout, ERHITextureLayout DstLayout, ERHIResourceAccess SrcAccess, ERHIResourceAccess DstAccess, ERHIPipelineStage SrcStage, ERHIPipelineStage DstStage, uint32_t SrcQueueFamilyIndex = ~(0U), uint32_t DstQueueFamilyIndex = ~(0U)) override;
    virtual void RHICmdDraw(FRHICommandList& RHICmdList, const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params) override;
    virtual void RHICmdDrawIndexed(FRHICommandList& RHICmdList, const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params) override;
    virtual void RHICmdDrawIndexed2(FRHICommandList& RHICmdList, const FRHIDrawCall& DrawCall) override;
    virtual void RHICmdBlitImage(FRHICommandList& RHICmdList, FRHITexture* Src, FRHITexture* Dst) override;
    virtual void RHICmdSetScissor(FRHICommandList& RHICmdList, int32_t X, int32_t Y, uint32_t Width, uint32_t Height) override;
    virtual std::shared_ptr<FRHIBuffer> RHIBindBuffer(void* Data, uint64_t Size) override;
    virtual FRHITexture* RHIGetBackBufferRenderTarget() const override;
    virtual std::array<uint32_t, 2> RHIGetViewportExtent() const override;
    // ~IRHI interface

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
    virtual VkSurfaceKHR RHIGetVkSurface() const override;
    virtual VkSwapchainKHR RHIGetVkSwapchain() const override;
    virtual VkExtent2D RHIGetSwapchainExtent() const override;
    virtual VkPresentModeKHR RHIGetSwapchainPresentMode() const override;
    virtual VkSurfaceFormatKHR RHIGetSwapchainSurfaceFormat() const override;
    virtual VkSemaphore GetImageAcquiredSemaphore() const override;
    virtual VkSemaphore GetRenderFinishedSemaphore() const override;
    virtual std::span<const char*> GetValidationExtensions() override;
    virtual std::span<const char*> GetInstanceExtensions() override;
    virtual std::span<const char*> GetPhysicalDeviceExtensions() override;
    virtual VkPipelineLayout GetBindlessPipelineLayout() const override;
    virtual VkDescriptorSet GetBindlessDescriptorSet() const override;
    virtual VkDescriptorSetLayout GetBindlessDesciptorSetLayout() const override;
    // ~IVulkanRHIMinimal interface

    // CVulkanRHI interface
    inline FVulkanDevice* GetDevice() const;
    inline RVulkanSwapchain* GetViewport() const;
    // ~CVulkanRHI interface

private:
    VkInstance Instance;

    FVulkanDevice* Device;
    RVulkanSwapchain* Swapchain;

    std::vector<const char*> InstanceExtensions;
    std::vector<const char*> ValidationLayerExtensions;
    std::vector<const char*> PhysicalDeviceExtensions;

    uint64_t FrameCounter;
};
