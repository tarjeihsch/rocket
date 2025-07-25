#pragma once

#include "vulkan/vulkan_core.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

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
    uint32_t Snapshot = 0;

    static VulkanShaderRef Capture(VulkanShader *pShader);
};

struct VulkanPipelineSpecification
{
    VkPipelineLayout Layout = VK_NULL_HANDLE;

    VulkanShaderRef VertexShader;
    VulkanShaderRef FragmentShader;

    std::vector<VkFormat> ColorAttachmentFormats;
    std::vector<VkVertexInputBindingDescription> VertexBindings;
    std::vector<VkVertexInputAttributeDescription> VertexAttributes;

    VkFormat DepthFormat = VK_FORMAT_UNDEFINED;
    VkFormat StencilFormat = VK_FORMAT_UNDEFINED;
    VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
    VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
    uint8_t DepthTestEnable;
    uint8_t DepthWriteEnable;
};

struct VulkanPipelineStateObject
{
    VulkanShaderRef VertexShader;
    VulkanShaderRef FragmentShader;
    VkPipeline Pipeline;
};

class FVulkanDevice;

class VulkanPipelineManager
{
public:
    explicit VulkanPipelineManager(FVulkanDevice& pDevice, VkPipelineCache pDriverCache = VK_NULL_HANDLE);
    ~VulkanPipelineManager();

    static VulkanPipelineSpecification MakeSpecification(VkPipelineLayout pLayout, VulkanShader *pVertexShader, VulkanShader *pFragmentShader);

    VkPipeline GetPipeline(const VulkanPipelineSpecification &pSpec);

    void GC();
    void Invalidate();

private:
    VkPipeline Compile(const VulkanPipelineSpecification &pSpec);
    uint64_t Hash(const VulkanPipelineSpecification &pSpec);

private:
    FVulkanDevice& Device;
    VkPipelineCache DriverCache;
    std::unordered_map<uint64_t, VulkanPipelineStateObject> Data;
};
