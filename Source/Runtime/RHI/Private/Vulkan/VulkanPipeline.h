#pragma once

#include "Common/RHIPipeline.h"

#include <unordered_map>
#include <vulkan/vulkan_core.h>

class VulkanShader
{
public:
    VulkanShader(VkDevice pDevice, VkShaderStageFlagBits pStage);

    void Create(const uint32_t *pData, const size_t pWordCount);
    void Destroy();

private:
    VkDevice Device;
    VkShaderStageFlagBits Stage;
    VkShaderModule Module;
    uint32_t Version;

    friend class VulkanShaderRef;
    friend class VulkanPipelineManager;
};

struct VulkanShaderRef
{
    VulkanShader *Shader; // this is a memory-leak
    uint32_t SnapshotVersion = 0;

    static VulkanShaderRef Capture(VulkanShader *pShader);
};

struct VulkanPipeline : RHIPipeline
{
    VulkanPipeline(VulkanShaderRef InVS, VulkanShaderRef InFS, VkPipeline InPipeline)
        : VertexShaderRef(std::move(InVS)), FragmentShaderRef(std::move(InFS)), Pipeline(InPipeline)
    {
    }

    virtual void* GetHandle() const override
    {
        return Pipeline;;
    }

    VulkanShaderRef VertexShaderRef;
    VulkanShaderRef FragmentShaderRef;
    VkPipeline Pipeline;
};

class VulkanDevice;

class VulkanPipelineManager
{
public:
    explicit VulkanPipelineManager(VulkanDevice& pDevice, VkPipelineCache pDriverCache = VK_NULL_HANDLE);
    ~VulkanPipelineManager();

    VulkanPipeline* GetOrCreatePipeline(const RHIPipelineDesc &pipeline_desc);

    void GC();
    void Invalidate();

private:
    VkPipeline Compile(const RHIPipelineDesc &Desc, VulkanShaderRef VertexShaderRef, VulkanShaderRef PixelShaderRef);
    uint64_t Hash(const RHIPipelineDesc &Desc);

private:
    VulkanDevice& Device;
    VkPipelineCache DriverCache;
    std::unordered_map<uint64_t, VulkanPipeline*> pipeline_cache;
};
