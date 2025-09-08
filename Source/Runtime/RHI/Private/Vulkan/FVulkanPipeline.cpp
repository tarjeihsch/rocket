#include "FVulkanPipeline.h"

#include "Assert.h"
#include "Logger.h"
#include "ShaderAsset.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanDescriptorManager.h"
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
inline void HashBytes(uint64_t& pHash, const void* pData, size_t pSize)
{
    const uint8_t* Data = reinterpret_cast<const uint8_t*>(pData);
    for (size_t Index = 0; Index < pSize; ++Index)
    {
        pHash ^= static_cast<uint64_t>(Data[Index]);
        pHash *= FNV_PRIME;
    }
}

template <typename TObject>
inline void HashObject(uint64_t& pHash, const TObject& pObject)
{
    static_assert(std::is_trivially_copyable_v<TObject>);
    auto bytes = std::as_bytes(std::span<const TObject>(&pObject, 1));
    HashBytes(pHash, bytes.data(), bytes.size());
}

inline void HashObject(uint64_t& pHash, std::string_view StringView)
{
    HashBytes(pHash, StringView.data(), StringView.size());
}

inline void HashObject(uint64_t& pHash, const char* StringLiteral)
{
    HashBytes(pHash, StringLiteral, std::strlen(StringLiteral));
}

FVulkanShader::FVulkanShader(VkDevice pDevice, VkShaderStageFlagBits pStage)
    : Device(pDevice), Stage(pStage), Module(VK_NULL_HANDLE), Version(0)
{
}

void FVulkanShader::Create(const uint32_t *pData, const size_t pWordCount)
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

void FVulkanShader::Destroy()
{
    if (Module)
    {
        VulkanRHI::vkDestroyShaderModule(Device, Module, nullptr);
    }

    Module = VK_NULL_HANDLE;
    Version = 0;
}

FVulkanShaderRef FVulkanShaderRef::Capture(FVulkanShader *pShader)
{
    FVulkanShaderRef Ref;
    Ref.Shader = pShader;
    Ref.SnapshotVersion = pShader->Version;
    return Ref;
}

FVulkanPipelineManager::FVulkanPipelineManager(FVulkanDevice& pDevice, VkPipelineCache pDriverCache)
    : Device(pDevice), DriverCache(pDriverCache)
{
}

FVulkanPipelineManager::~FVulkanPipelineManager()
{
    Invalidate();
}

FVulkanPipeline* FVulkanPipelineManager::GetPipeline(const FRHIPipelineDesc &Desc)
{
    const uint64_t Key = Hash(Desc);
    if (auto It = Data.find(Key); It != Data.end())
    {
        return It->second;
    }

    FAssetLoaderRequest VSRequest = { Desc.VertexShaderPath };
    ShaderAssetOptions VSOptions = { "main", "vs_6_6" };
    FAssetHandle<ShaderAsset> VertexShader = FAssetManager::Get().Load<ShaderAsset>(VSRequest, VSOptions);
    ShaderAsset* VertexShaderAsset = FAssetManager::Get().GetAsset(VertexShader);

    FAssetLoaderRequest PSRequest = { Desc.PixelShaderPath };
    ShaderAssetOptions PSOptions = { "main", "ps_6_6" };
    FAssetHandle<ShaderAsset> PixelShader = FAssetManager::Get().Load<ShaderAsset>(PSRequest, PSOptions);
    ShaderAsset* PixelShaderAsset = FAssetManager::Get().GetAsset(PixelShader);

    FVulkanShaderRef VertexShaderRef = FVulkanShaderRef::Capture(new FVulkanShader(Device.GetVkDevice(), VK_SHADER_STAGE_VERTEX_BIT));
    FVulkanShaderRef PixelShaderRef = FVulkanShaderRef::Capture(new FVulkanShader(Device.GetVkDevice(), VK_SHADER_STAGE_FRAGMENT_BIT));

    VertexShaderRef.Shader->Create(VertexShaderAsset->Data(), VertexShaderAsset->WordCount());
    PixelShaderRef.Shader->Create(PixelShaderAsset->Data(), PixelShaderAsset->WordCount());

    FAssetManager::Get().Unload(VertexShader);
    FAssetManager::Get().Unload(PixelShader);

    VkPipeline Pipeline = Compile(Desc, VertexShaderRef, PixelShaderRef);
    FVulkanPipeline* PSO = new FVulkanPipeline(VertexShaderRef, PixelShaderRef, Pipeline);
    Data.emplace(Key, PSO);
    return PSO;
}

void FVulkanPipelineManager::GC()
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

void FVulkanPipelineManager::Invalidate()
{
    for (auto& KV : Data)
    {
        VulkanRHI::vkDestroyPipeline(Device.GetVkDevice(), KV.second->Pipeline, nullptr);
        VulkanRHI::vkDestroyShaderModule(Device.GetVkDevice(), KV.second->VertexShaderRef.Shader->Module, nullptr);
        VulkanRHI::vkDestroyShaderModule(Device.GetVkDevice(), KV.second->FragmentShaderRef.Shader->Module, nullptr);
    }
    Data.clear();
}

VkPipeline FVulkanPipelineManager::Compile(const FRHIPipelineDesc &Desc, FVulkanShaderRef VertexShaderRef, FVulkanShaderRef PixelShaderRef)
{
    std::vector<VkPipelineShaderStageCreateInfo> ShaderStages = {};

    VkPipelineShaderStageCreateInfo VSElement = {};
    VSElement.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    VSElement.stage = VertexShaderRef.Shader->Stage;
    VSElement.module = VertexShaderRef.Shader->Module;
    VSElement.pName = "main";
    ShaderStages.push_back(VSElement);

    VkPipelineShaderStageCreateInfo FSElement = {};
    FSElement.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    FSElement.stage = PixelShaderRef.Shader->Stage;
    FSElement.module = PixelShaderRef.Shader->Module;
    FSElement.pName = "main";
    ShaderStages.push_back(FSElement);

    std::vector<FRHIVertexBinding> VertexBindings;
    std::vector<FRHIVertexAttribute> VertexAttributes;

    VertexBindings.push_back(FRHIVertexBinding(0, 48, 0));
    VertexAttributes.push_back(FRHIVertexAttribute(0, 0, ERHIFormat::Float3, offsetof(FRHIVertex, m_Position)));
    VertexAttributes.push_back(FRHIVertexAttribute(1, 0, ERHIFormat::Float3, offsetof(FRHIVertex, m_Normal)));
    VertexAttributes.push_back(FRHIVertexAttribute(2, 0, ERHIFormat::Float2, offsetof(FRHIVertex, m_TexCoord)));

    std::vector<VkVertexInputBindingDescription> VertexBindingsDescriptions;
    for (const FRHIVertexBinding& Binding : VertexBindings)
    {
        VkVertexInputBindingDescription VkBinding = {};
        VkBinding.binding = Binding.Binding;
        VkBinding.stride = Binding.Stride;
        VkBinding.inputRate = Binding.bPerInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
        VertexBindingsDescriptions.push_back(VkBinding);
    }

    std::vector<VkVertexInputAttributeDescription> VertexAttributeDescriptions;
    for (const FRHIVertexAttribute& Attr : VertexAttributes)
    {
        VkVertexInputAttributeDescription VkAttr = {};
        VkAttr.location = Attr.Location;
        VkAttr.binding = Attr.Binding;
        VkAttr.format = VulkanRHI::ToVkFormat(Attr.Format);
        VkAttr.offset = Attr.Offset;
        VertexAttributeDescriptions.push_back(VkAttr);
    }

    std::vector<VkPipelineColorBlendAttachmentState> BlendAttachments(Desc.ColorAttachmentFormats.size());
    for (auto& BlendAttachment : BlendAttachments)
    {
        BlendAttachment.blendEnable = VK_FALSE;

        BlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        BlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        BlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

        BlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        BlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        BlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        BlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }

    VkPipelineColorBlendStateCreateInfo ColorBlendCreateInfo = {};
    ColorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendCreateInfo.attachmentCount = static_cast<uint32_t>(BlendAttachments.size());
    ColorBlendCreateInfo.pAttachments = BlendAttachments.data();

    VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo = {};
    VertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(VertexBindingsDescriptions.size());
    VertexInputStateCreateInfo.pVertexBindingDescriptions = VertexBindingsDescriptions.data();
    VertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(VertexAttributeDescriptions.size());
    VertexInputStateCreateInfo.pVertexAttributeDescriptions = VertexAttributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo = {};
    InputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyCreateInfo.topology = VulkanRHI::ToVkTopology(Desc.Topology);

    VkPipelineRasterizationStateCreateInfo RasterizerCreateInfo = {};
    RasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizerCreateInfo.polygonMode = VulkanRHI::ToVkPolygonMode(Desc.PolygonMode);
    RasterizerCreateInfo.cullMode = VulkanRHI::ToVkCullMode(Desc.CullMode);
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
    DepthStencilCreateInfo.depthTestEnable = Desc.bDepthTestEnable ? VK_TRUE : VK_FALSE;
    DepthStencilCreateInfo.depthWriteEnable = Desc.bDepthWriteEnable ? VK_TRUE : VK_FALSE;
    DepthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    std::vector<VkFormat> ColorAttachmentFormats;
    for (ERHIFormat Format : Desc.ColorAttachmentFormats)
    {
        ColorAttachmentFormats.push_back(VulkanRHI::ToVkFormat(Format));
    }

    VkPipelineRenderingCreateInfo RenderingCreateInfo = {};
    RenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    RenderingCreateInfo.pColorAttachmentFormats = ColorAttachmentFormats.data();
    RenderingCreateInfo.colorAttachmentCount = static_cast<uint32_t>(ColorAttachmentFormats.size());
    RenderingCreateInfo.depthAttachmentFormat = VulkanRHI::ToVkFormat(Desc.DepthFormat);
    RenderingCreateInfo.stencilAttachmentFormat = VulkanRHI::ToVkFormat(Desc.StencilFormat);

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
    PipelineCreateInfo.pStages = ShaderStages.data();
    PipelineCreateInfo.stageCount = ShaderStages.size();
    PipelineCreateInfo.pVertexInputState = &VertexInputStateCreateInfo;
    PipelineCreateInfo.pInputAssemblyState = &InputAssemblyCreateInfo;
    PipelineCreateInfo.pRasterizationState = &RasterizerCreateInfo;
    PipelineCreateInfo.pViewportState = &ViewportStateCreateInfo;
    PipelineCreateInfo.pMultisampleState = &MultisampleCreateInfo;
    PipelineCreateInfo.pColorBlendState = &ColorBlendCreateInfo;
    PipelineCreateInfo.pDepthStencilState = &DepthStencilCreateInfo;
    PipelineCreateInfo.pDynamicState = &DynamicStateCreateInfo;
    PipelineCreateInfo.layout = Device.GetDescriptorManager()->GetPipelineLayout();

    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkResult Result = VulkanRHI::vkCreateGraphicsPipelines(Device.GetVkDevice(), DriverCache, 1, &PipelineCreateInfo, nullptr, &Pipeline);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create graphics pipeline");

    return Pipeline;
}

uint64_t FVulkanPipelineManager::Hash(const FRHIPipelineDesc &P_Spec)
{
    uint64_t Hash = FNV_OFFSET;

    HashObject(Hash, P_Spec.VertexShaderPath);
    HashObject(Hash, P_Spec.PixelShaderPath);
    HashObject(Hash, P_Spec.DepthFormat);
    HashObject(Hash, P_Spec.StencilFormat);
    HashObject(Hash, P_Spec.Topology);
    HashObject(Hash, P_Spec.CullMode);
    HashObject(Hash, P_Spec.PolygonMode);
    HashObject(Hash, P_Spec.bDepthTestEnable);
    HashObject(Hash, P_Spec.bDepthWriteEnable);
    //HashBytes(Hash, P_Spec.ColorAttachmentFormats.data(), P_Spec.ColorAttachmentFormats.size() * sizeof(ERHIFormat));
    //HashBytes(Hash, P_Spec.VertexBindings.data(), P_Spec.VertexBindings.size() * sizeof(FRHIVertexBinding));
    //HashBytes(Hash, P_Spec.VertexAttributes.data(), P_Spec.VertexAttributes.size() * sizeof(FRHIVertexAttribute));

    return Hash;
}
