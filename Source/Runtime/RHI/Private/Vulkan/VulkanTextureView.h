#pragma once

#include "Common/RHITextureView.h"
#include "VulkanDescriptorManager.h"

class VulkanDevice;

struct VulkanTextureView : public RHITextureView
{
private:
	VulkanDevice& Device;

public:
	VulkanTextureView(VulkanDevice& InDevice, const RHITextureViewDesc& InDesc, RHITexture* Texture);
	VulkanTextureView(VulkanDevice& InDevice, const RHITextureViewDesc& InDesc, RHITexture* Texture, VkImageView InImageView);
	~VulkanTextureView();

	virtual void* GetNativeHandle() const override;
	virtual const RHITextureViewDesc& GetDesc() const override;
	virtual RHIDescriptorHandle GetDescriptorHandle() const override;
	virtual RHITexture* GetRHITexture() const override;

	VkImageView NativeHandle;
	RHITexture* target_texture;

private:
	void Alloc();
	void Free();

private:
	RHITextureViewDesc desc;
	
	RHIDescriptorHandle DescriptorHandle;
};