#include "FVulkanPipeline.h"

#include "Assert.h"
#include "Logger.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"

#include <vulkan/vulkan_core.h>

#ifdef RK_PLATFORM_LINUX
    #include <dlfcn.h>
    #include <dxc/dxcapi.h>
#endif

static constexpr uint64_t FNV_OFFSET = 1469598103934665603ull;
static constexpr uint64_t FNV_PRIME  = 1099511628211ull;

// FNV-1a 64-bit
static inline void HashBytes(uint64_t& pHash, const void* pData, size_t pSize)
{
    const uint8_t* Data = reinterpret_cast<const uint8_t*>(pData);
    for (size_t Index = 0; Index < pSize; ++Index)
    {
        pHash ^= static_cast<uint64_t>(Data[Index]);
        pHash *= FNV_PRIME;
    }
}

template <typename TObject>
static inline void HashObject(uint64_t& pHash, const TObject& pObject)
{
    static_assert(std::is_trivially_copyable_v<TObject>);
    auto bytes = std::as_bytes(std::span<const TObject>(&pObject, 1));
    HashBytes(pHash, bytes.data(), bytes.size());
}

VulkanShader::VulkanShader(VkDevice pDevice, VkShaderStageFlagBits pStage)
    : Device(pDevice), Stage(pStage), Module(VK_NULL_HANDLE), Version(0)
{
}

void VulkanShader::Create(const uint32_t *pData, const size_t pWordCount)
{
    if (Module)
    {
        RK_LOG_INFO("Shader Module hot-reload");

        VulkanRHI::vkDestroyShaderModule(Device, Module, nullptr);
    }

    VkShaderModuleCreateInfo CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    CreateInfo.pCode = pData;
    CreateInfo.codeSize = pWordCount * sizeof(uint32_t);
    VkResult Result = VulkanRHI::vkCreateShaderModule(Device, &CreateInfo, nullptr, &Module);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create shader module.");

    Version += 1;
}

void VulkanShader::Destroy()
{
    if (Module)
    {
        VulkanRHI::vkDestroyShaderModule(Device, Module, nullptr);
    }

    Module = VK_NULL_HANDLE;
    Version = 0;
}

VulkanShaderRef VulkanShaderRef::Capture(VulkanShader *pShader)
{
    VulkanShaderRef Ref;
    Ref.Shader = pShader;
    Ref.SnapshotVersion = pShader->Version;
    return Ref;
}

VulkanPipelineManager::VulkanPipelineManager(FVulkanDevice& pDevice, VkPipelineCache pDriverCache)
    : Device(pDevice), DriverCache(pDriverCache)
{
}

VulkanPipelineManager::~VulkanPipelineManager()
{
    Invalidate();
}

VulkanPipelineSpecification VulkanPipelineManager::MakeSpecification(VkPipelineLayout pLayout, VulkanShader *pVertexShader, VulkanShader *pFragmentShader)
{
    VulkanPipelineSpecification Spec = {};
    Spec.Layout = pLayout;
    Spec.VertexShader = VulkanShaderRef::Capture(pVertexShader);
    Spec.FragmentShader = VulkanShaderRef::Capture(pFragmentShader);
    return Spec;
}

FVulkanPipeline* VulkanPipelineManager::GetPipeline(const VulkanPipelineSpecification &pSpec)
{
    const uint64_t Key = Hash(pSpec);
    if (auto It = Data.find(Key); It != Data.end())
    {
        return It->second;
    }

    VkPipeline Pipeline = Compile(pSpec);
    FVulkanPipeline* PSO = new FVulkanPipeline(
        pSpec.VertexShader,
        pSpec.FragmentShader,
        Pipeline
    );
    Data.emplace(Key, PSO);
    return PSO;
}

void VulkanPipelineManager::GC()
{
    for (auto It = Data.begin(); It != Data.end(); )
    {
        bool Stale = false;

        const FVulkanPipeline& PSO = *It->second;

        if (PSO.VertexShaderRef.SnapshotVersion != PSO.VertexShaderRef.Shader->Version)
        {
            Stale = true;
        }

        if (PSO.FragmentShaderRef.SnapshotVersion != PSO.FragmentShaderRef.Shader->Version)
        {
            Stale = true;
        }

        if (Stale)
        {
            VulkanRHI::vkDestroyPipeline(Device.GetVkDevice(), It->second->Pipeline, nullptr);
            It = Data.erase(It);
        }
        else
        {
            ++It;
        }
    }
}

void VulkanPipelineManager::Invalidate()
{
    for (auto& KV : Data)
    {
        VulkanRHI::vkDestroyPipeline(Device.GetVkDevice(), KV.second->Pipeline, nullptr);
        VulkanRHI::vkDestroyShaderModule(Device.GetVkDevice(), KV.second->VertexShaderRef.Shader->Module, nullptr);
        VulkanRHI::vkDestroyShaderModule(Device.GetVkDevice(), KV.second->FragmentShaderRef.Shader->Module, nullptr);
    }
    Data.clear();
}

VkPipeline VulkanPipelineManager::Compile(const VulkanPipelineSpecification &pSpec)
{
    VkPipelineShaderStageCreateInfo ShaderStages[2] = {};
    uint32_t StageCount = 0;
    if (pSpec.VertexShader.Shader && pSpec.VertexShader.Shader->Module)
    {
        auto& VSElement = ShaderStages[StageCount++];
        VSElement.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        VSElement.stage = pSpec.VertexShader.Shader->Stage;
        VSElement.module = pSpec.VertexShader.Shader->Module;
        VSElement.pName = "main";
    }

    if (pSpec.FragmentShader.Shader && pSpec.FragmentShader.Shader->Module)
    {
        auto& FSElement = ShaderStages[StageCount++];
        FSElement.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        FSElement.stage = pSpec.FragmentShader.Shader->Stage;
        FSElement.module = pSpec.FragmentShader.Shader->Module;
        FSElement.pName = "main";
    }

    VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo = {};
    VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(pSpec.VertexBindings.size());
    VertexInputStateCreateInfo.pVertexBindingDescriptions = pSpec.VertexBindings.data();
    VertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(pSpec.VertexAttributes.size());
    VertexInputStateCreateInfo.pVertexAttributeDescriptions = pSpec.VertexAttributes.data();

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo = {};
    InputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyCreateInfo.topology = pSpec.Topology;

    VkPipelineRasterizationStateCreateInfo RasterizerCreateInfo = {};
    RasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizerCreateInfo.polygonMode = pSpec.PolygonMode;
    RasterizerCreateInfo.cullMode = pSpec.CullMode;
    RasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    RasterizerCreateInfo.lineWidth = 1.0f;

    VkPipelineViewportStateCreateInfo ViewportStateCreateInfo = {};
    ViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportStateCreateInfo.pNext = nullptr;
    ViewportStateCreateInfo.viewportCount = 1;
    ViewportStateCreateInfo.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo MultisampleCreateInfo = {};
    MultisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo = {};
    DepthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthStencilCreateInfo.depthTestEnable = pSpec.DepthTestEnable ? VK_TRUE : VK_FALSE;
    DepthStencilCreateInfo.depthWriteEnable = pSpec.DepthWriteEnable ? VK_TRUE : VK_FALSE;
    DepthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    std::vector<VkPipelineColorBlendAttachmentState> BlendAttachments(pSpec.ColorAttachmentFormats.size());
    for (auto& BlendAttachment : BlendAttachments)
    {
        BlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        BlendAttachment.blendEnable = VK_FALSE;
    }

    VkPipelineColorBlendStateCreateInfo ColorBlendCreateInfo = {};
    ColorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendCreateInfo.attachmentCount = static_cast<uint32_t>(BlendAttachments.size());
    ColorBlendCreateInfo.pAttachments = BlendAttachments.data();

    VkPipelineRenderingCreateInfo RenderingCreateInfo = {};
    RenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    RenderingCreateInfo.colorAttachmentCount = static_cast<uint32_t>(pSpec.ColorAttachmentFormats.size());
    RenderingCreateInfo.pColorAttachmentFormats = pSpec.ColorAttachmentFormats.data();
    RenderingCreateInfo.depthAttachmentFormat = pSpec.DepthFormat;
    RenderingCreateInfo.stencilAttachmentFormat = pSpec.StencilFormat;

    std::vector<VkDynamicState> DynamicStates = {};
    DynamicStates.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
    DynamicStates.emplace_back(VK_DYNAMIC_STATE_SCISSOR);

    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo {};
    DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(DynamicStates.size());
    DynamicStateCreateInfo.pDynamicStates = DynamicStates.data();

    VkGraphicsPipelineCreateInfo PipelineCreateInfo = {};
    PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCreateInfo.pNext = &RenderingCreateInfo;
    PipelineCreateInfo.pStages = ShaderStages;
    PipelineCreateInfo.stageCount = 2;
    PipelineCreateInfo.pVertexInputState = &VertexInputStateCreateInfo;
    PipelineCreateInfo.pInputAssemblyState = &InputAssemblyCreateInfo;
    PipelineCreateInfo.pRasterizationState = &RasterizerCreateInfo;
    PipelineCreateInfo.pViewportState = &ViewportStateCreateInfo;
    PipelineCreateInfo.pMultisampleState = &MultisampleCreateInfo;
    PipelineCreateInfo.pColorBlendState = &ColorBlendCreateInfo;
    PipelineCreateInfo.pDepthStencilState = &DepthStencilCreateInfo;
    PipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;
    PipelineCreateInfo.layout = pSpec.Layout;

    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkResult Result = VulkanRHI::vkCreateGraphicsPipelines(Device.GetVkDevice(), DriverCache, 1, &PipelineCreateInfo, nullptr, &Pipeline);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create graphics pipeline");

    return Pipeline;
}

uint64_t VulkanPipelineManager::Hash(const VulkanPipelineSpecification &pSpec)
{
    uint64_t Hash = FNV_OFFSET;

    HashObject(Hash, pSpec.Layout);
    HashObject(Hash, pSpec.VertexShader.Shader);
    HashObject(Hash, pSpec.VertexShader.SnapshotVersion);
    HashObject(Hash, pSpec.FragmentShader.Shader);
    HashObject(Hash, pSpec.FragmentShader.SnapshotVersion);
    HashObject(Hash, pSpec.DepthFormat);
    HashObject(Hash, pSpec.StencilFormat);
    HashObject(Hash, pSpec.Topology);
    HashObject(Hash, pSpec.CullMode);
    HashObject(Hash, pSpec.PolygonMode);
    HashObject(Hash, pSpec.DepthTestEnable);
    HashObject(Hash, pSpec.DepthWriteEnable);
    HashBytes(Hash, pSpec.ColorAttachmentFormats.data(), pSpec.ColorAttachmentFormats.size() * sizeof(VkFormat));
    HashBytes(Hash, pSpec.VertexBindings.data(), pSpec.VertexBindings.size() * sizeof(VkVertexInputBindingDescription));
    HashBytes(Hash, pSpec.VertexAttributes.data(), pSpec.VertexAttributes.size() * sizeof(VkVertexInputAttributeDescription));

    return Hash;
}