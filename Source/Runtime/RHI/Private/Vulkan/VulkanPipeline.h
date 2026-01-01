#pragma once

#include "Common/RHIPipeline.h"

#include <unordered_map>
#include <vulkan/vulkan_core.h>

class FVulkanShader
{
public:
    FVulkanShader(VkDevice pDevice, VkShaderStageFlagBits pStage);

    void Create(const uint32_t *pData, const size_t pWordCount);
    void Destroy();

private:
    VkDevice Device;
    VkShaderStageFlagBits Stage;
    VkShaderModule Module;
    uint32_t Version;

    friend class FVulkanShaderRef;
    friend class FVulkanPipelineManager;
};

struct FVulkanShaderRef
{
    FVulkanShader *Shader; // this is a memory-leak
    uint32_t SnapshotVersion = 0;

    static FVulkanShaderRef Capture(FVulkanShader *pShader);
};

struct FVulkanPipeline : RHIPipeline
{
    FVulkanPipeline(FVulkanShaderRef InVS, FVulkanShaderRef InFS, VkPipeline InPipeline)
        : VertexShaderRef(std::move(InVS)), FragmentShaderRef(std::move(InFS)), Pipeline(InPipeline)
    {
    }

    virtual void* GetHandle() const override
    {
        return Pipeline;;
    }

    FVulkanShaderRef VertexShaderRef;
    FVulkanShaderRef FragmentShaderRef;
    VkPipeline Pipeline;
};

class VulkanDevice;

class FVulkanPipelineManager
{
public:
    explicit FVulkanPipelineManager(VulkanDevice& pDevice, VkPipelineCache pDriverCache = VK_NULL_HANDLE);
    ~FVulkanPipelineManager();

    FVulkanPipeline* get_pipeline(const RHIPipelineDesc &Desc);

    void GC();
    void Invalidate();

private:
    VkPipeline Compile(const RHIPipelineDesc &Desc, FVulkanShaderRef VertexShaderRef, FVulkanShaderRef PixelShaderRef);
    uint64_t Hash(const RHIPipelineDesc &Desc);

private:
    VulkanDevice& Device;
    VkPipelineCache DriverCache;
    std::unordered_map<uint64_t, FVulkanPipeline*> Data;
};
