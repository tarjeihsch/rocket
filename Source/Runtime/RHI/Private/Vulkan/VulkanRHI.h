#pragma once

#include "Vulkan/VulkanRHIMinimal.h"

#include <vector>

struct FRHITextureView;
struct FRHITextureAttachment;
class FVulkanDevice;
class VulkanSwapchain;
class FrameGraph;

class CVulkanRHI : public IVulkanRHIMinimal
{
public:
    // IRHI interface
    virtual void Init() override;
    virtual void Shutdown() override;

    virtual void submit() override;
    virtual void reset() override;

    [[nodiscard]] virtual const char* GetName() const override;
    [[nodiscard]] virtual const char* GetVersion() const override;
    [[nodiscard]] virtual ERHIInterfaceType GetInterfaceType() const noexcept override;
    [[nodiscard]] virtual IRHI* GetNonValidationRHI() const noexcept override;

    virtual RHISurface* create_surface(RHISurfaceDesc& desc) override;
    virtual void RHIWaitUntilIdle() const override;
    virtual uint64_t GetFrameCounter() const noexcept override;
    virtual RHITexture* RHICreateTexture(FRHITextureDesc Desc) const override;
    virtual FRHITextureView* RHICreateTextureView(FRHITextureViewDesc TextureViewDesc, RHITexture* Texture) const override;
    virtual FRHIBuffer* RHICreateBuffer(FRHIBufferDesc Desc) const override;
    virtual RRHISampler* RHICreateSampler(RRHISamplerDesc Desc) const override;
    virtual void RHIUploadDataBlocking(FRHIBuffer* Buffer, void* Data, uint64_t Size) override;
    virtual void RHIUploadTextureBlocking(RHITexture* Texture2D, const uint8_t* Data, uint32_t Width, uint32_t Height) override;
    virtual FRHIPipeline* RHICreatePipelineStateObject(FRHIPipelineDesc Desc) override;
    virtual IRHICommandListContext* GetCommandListContext() const override;
    virtual void RHICmdBindPipeline(RHICommandList& RHICmdList, FRHIPipeline* Pipeline) override;
    virtual void RHICmdBeginRendering(RHICommandList* RHICmdList, std::span<FRHITextureView*> ColorAttachments, FRHITextureView* DepthAttachmentView, bool bClearColor) override;
    virtual void RHICmdEndRendering(RHICommandList* RHICmdList) override;
    virtual void RHICmdPipelineBarrier(RHICommandList* RHICmdList, RHITexture* Texture, ERHITextureLayout SrcLayout, ERHITextureLayout DstLayout, ERHIResourceAccess SrcAccess, ERHIResourceAccess DstAccess, ERHIPipelineStage SrcStage, ERHIPipelineStage DstStage, uint32_t SrcQueueFamilyIndex = ~(0U), uint32_t DstQueueFamilyIndex = ~(0U)) override;
    virtual void RHICmdDraw(RHICommandList& RHICmdList, const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params) override;
    virtual void RHICmdDrawIndexed(RHICommandList& RHICmdList, const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params) override;
    virtual void RHICmdDrawIndexed2(RHICommandList& RHICmdList, const FRHIDrawCall& DrawCall) override;
    virtual void RHICmdBlitImage(RHICommandList& RHICmdList, RHITexture* Src, RHITexture* Dst) override;
    virtual void RHICmdSetScissor(RHICommandList& RHICmdList, int32_t X, int32_t Y, uint32_t Width, uint32_t Height) override;
    virtual std::shared_ptr<FRHIBuffer> RHIBindBuffer(void* Data, uint64_t Size) override;
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
    virtual std::span<const char*> GetValidationExtensions() override;
    virtual std::span<const char*> GetInstanceExtensions() override;
    virtual std::span<const char*> GetPhysicalDeviceExtensions() override;
    virtual VkPipelineLayout GetBindlessPipelineLayout() const override;
    virtual VkDescriptorSet GetBindlessDescriptorSet() const override;
    virtual VkDescriptorSetLayout GetBindlessDesciptorSetLayout() const override;
    // ~IVulkanRHIMinimal interface

    // CVulkanRHI interface
    inline FVulkanDevice* GetDevice() const;
    // ~CVulkanRHI interface

    // Call Renderer::Begin()
    //TDelegate<> BeginFrame;

    // Call Renderer::End()
    //TDelegate<> EndFrame;

private:
    VkInstance Instance;
    FVulkanDevice* Device;

    std::vector<const char*> InstanceExtensions;
    std::vector<const char*> ValidationLayerExtensions;
    std::vector<const char*> PhysicalDeviceExtensions;

    uint64_t FrameCounter;
};
