#include "VulkanPipeline.h"

#include "Assertion.h"
#include "Logger.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanDescriptorManager.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"

#include <fstream>

#ifdef RK_PLATFORM_LINUX
    #include <dlfcn.h>
    #include <dxc/dxcapi.h>
#endif

static std::vector<uint32_t> LoadFile(const ANSICHAR* file_path)
{
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);

    size_t file_size = file.tellg();
    std::vector<uint32_t> data(file_size / sizeof(uint32_t));

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(data.data()), file_size);

    return data;
}

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
    ensure(Result == VK_SUCCESS, "Failed to create shader module.");

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

VulkanPipelineManager::VulkanPipelineManager(VulkanDevice& pDevice, VkPipelineCache pDriverCache)
    : Device(pDevice), DriverCache(pDriverCache)
{
}

VulkanPipelineManager::~VulkanPipelineManager()
{
    Invalidate();
}

VulkanPipeline* VulkanPipelineManager::GetOrCreatePipeline(const RHIPipelineDesc &pipeline_desc)
{
    const uint64_t key = Hash(pipeline_desc);
    if (auto It = pipeline_cache.find(key); It != pipeline_cache.end())
    {
        return It->second;
    }

    std::vector<uint32_t> vertex_shader_blob = LoadFile(pipeline_desc.vertex_shader_path);
    std::vector<uint32_t> pixel_shader_blob = LoadFile(pipeline_desc.pixel_shader_path);

    VulkanShaderRef vertex_shader_handle = VulkanShaderRef::Capture(new VulkanShader(Device.GetVkDevice(), VK_SHADER_STAGE_VERTEX_BIT));
    VulkanShaderRef pixel_shader_handle = VulkanShaderRef::Capture(new VulkanShader(Device.GetVkDevice(), VK_SHADER_STAGE_FRAGMENT_BIT));

    vertex_shader_handle.Shader->Create(vertex_shader_blob.data(), vertex_shader_blob.size());
    pixel_shader_handle.Shader->Create(pixel_shader_blob.data(), pixel_shader_blob.size());

    VkPipeline pipeline_handle = Compile(pipeline_desc, vertex_shader_handle, pixel_shader_handle);
    VulkanPipeline* pipeline = new VulkanPipeline(vertex_shader_handle, pixel_shader_handle, pipeline_handle);
    pipeline_cache.emplace(key, pipeline);
    return pipeline;

    //FAssetLoaderRequest VSRequest = { Desc.VertexShaderPath };
    //ShaderAssetOptions VSOptions = { "main", "vs_6_6" };
    //FAssetHandle<ShaderAsset> VertexShader = FAssetManager::Get().Load<ShaderAsset>(VSRequest, VSOptions);
    //ShaderAsset* VertexShaderAsset = FAssetManager::Get().GetAsset(VertexShader);
    //
    //FAssetLoaderRequest PSRequest = { Desc.PixelShaderPath };
    //ShaderAssetOptions PSOptions = { "main", "ps_6_6" };
    //FAssetHandle<ShaderAsset> PixelShader = FAssetManager::Get().Load<ShaderAsset>(PSRequest, PSOptions);
    //ShaderAsset* PixelShaderAsset = FAssetManager::Get().GetAsset(PixelShader);
    //
    //FVulkanShaderRef VertexShaderRef = FVulkanShaderRef::Capture(new FVulkanShader(Device.GetVkDevice(), VK_SHADER_STAGE_VERTEX_BIT));
    //FVulkanShaderRef PixelShaderRef = FVulkanShaderRef::Capture(new FVulkanShader(Device.GetVkDevice(), VK_SHADER_STAGE_FRAGMENT_BIT));
    //
    //VertexShaderRef.Shader->Create(VertexShaderAsset->Data(), VertexShaderAsset->WordCount());
    //PixelShaderRef.Shader->Create(PixelShaderAsset->Data(), PixelShaderAsset->WordCount());
    //
    //FAssetManager::Get().Unload(VertexShader);
    //FAssetManager::Get().Unload(PixelShader);
    //
    //VkPipeline Pipeline = Compile(Desc, VertexShaderRef, PixelShaderRef);
    //FVulkanPipeline* PSO = new FVulkanPipeline(VertexShaderRef, PixelShaderRef, Pipeline);
    //Data.emplace(Key, PSO);
    //return PSO;
    return nullptr;
}

void VulkanPipelineManager::GC()
{
    for (auto It = pipeline_cache.begin(); It != pipeline_cache.end(); )
    {
        bool Stale = false;

        const VulkanPipeline& PSO = *It->second;

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
            It = pipeline_cache.erase(It);
        }
        else
        {
            ++It;
        }
    }
}

void VulkanPipelineManager::Invalidate()
{
    for (auto& KV : pipeline_cache)
    {
        VulkanRHI::vkDestroyPipeline(Device.GetVkDevice(), KV.second->Pipeline, nullptr);
        VulkanRHI::vkDestroyShaderModule(Device.GetVkDevice(), KV.second->VertexShaderRef.Shader->Module, nullptr);
        VulkanRHI::vkDestroyShaderModule(Device.GetVkDevice(), KV.second->FragmentShaderRef.Shader->Module, nullptr);
    }
    pipeline_cache.clear();
}

VkPipeline VulkanPipelineManager::Compile(const RHIPipelineDesc &Desc, VulkanShaderRef VertexShaderRef, VulkanShaderRef PixelShaderRef)
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

    std::vector<VkVertexInputBindingDescription> VertexBindingsDescriptions;
    for (const RHIVertexBinding& Binding : Desc.VertexBindings)
    {
        VkVertexInputBindingDescription& VkBinding = VertexBindingsDescriptions.emplace_back();
        VkBinding.binding = Binding.binding;
        VkBinding.stride = Binding.stride;
        VkBinding.inputRate = Binding.instance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
    }

    std::vector<VkVertexInputAttributeDescription> VertexAttributeDescriptions;
    for (const RHIVertexAttribute& Attr : Desc.VertexAttributes)
    {
        VkVertexInputAttributeDescription& VkAttr = VertexAttributeDescriptions.emplace_back();
        VkAttr.location = Attr.Location;
        VkAttr.binding = Attr.Binding;
        VkAttr.format = VulkanRHI::ToVkFormat(Attr.Format);
        VkAttr.offset = Attr.Offset;
    }

    std::vector<VkPipelineColorBlendAttachmentState> BlendAttachments;
    for (const RHIBlendAttachment& BlendAttachment : Desc.BlendAttachments)
    {
        VkPipelineColorBlendAttachmentState& VkBlendAttachment = BlendAttachments.emplace_back();
        VkBlendAttachment.blendEnable = static_cast<VkBool32>(BlendAttachment.bEnableBlend);
        VkBlendAttachment.srcColorBlendFactor = VulkanRHI::GetVkBlendFactor(BlendAttachment.SrcColorFactor);
        VkBlendAttachment.dstColorBlendFactor = VulkanRHI::GetVkBlendFactor(BlendAttachment.DstColorFactor);
        VkBlendAttachment.colorBlendOp = VulkanRHI::GetVkBlendOp(BlendAttachment.ColorOp);

        VkBlendAttachment.srcAlphaBlendFactor = VulkanRHI::GetVkBlendFactor(BlendAttachment.SrcAlphaFactor);
        VkBlendAttachment.dstAlphaBlendFactor = VulkanRHI::GetVkBlendFactor(BlendAttachment.DstAlphaFactor);
        VkBlendAttachment.alphaBlendOp = VulkanRHI::GetVkBlendOp(BlendAttachment.AlphaOp);

        VkBlendAttachment.colorWriteMask = 0;
        if (BlendAttachment.ColorWriteMask & (1 << 0)) VkBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
        if (BlendAttachment.ColorWriteMask & (1 << 1)) VkBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
        if (BlendAttachment.ColorWriteMask & (1 << 2)) VkBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
        if (BlendAttachment.ColorWriteMask & (1 << 3)) VkBlendAttachment.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;
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
    InputAssemblyCreateInfo.topology = VulkanRHI::ToVkTopology(Desc.topology);

    VkPipelineRasterizationStateCreateInfo RasterizerCreateInfo = {};
    RasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizerCreateInfo.polygonMode = VulkanRHI::ToVkPolygonMode(Desc.polygon_mode);
    RasterizerCreateInfo.cullMode = VulkanRHI::ToVkCullMode(Desc.cull_mode);
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
    DepthStencilCreateInfo.depthTestEnable = Desc.depth_test_enable ? VK_TRUE : VK_FALSE;
    DepthStencilCreateInfo.depthWriteEnable = Desc.depth_write_enable ? VK_TRUE : VK_FALSE;
    DepthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;

    std::vector<VkFormat> ColorAttachmentFormats;
    for (RHIFormat Format : Desc.color_attachment_formats)
    {
        ColorAttachmentFormats.push_back(VulkanRHI::ToVkFormat(Format));
    }

    VkPipelineRenderingCreateInfo RenderingCreateInfo = {};
    RenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    RenderingCreateInfo.pColorAttachmentFormats = ColorAttachmentFormats.data();
    RenderingCreateInfo.colorAttachmentCount = static_cast<uint32_t>(ColorAttachmentFormats.size());
    RenderingCreateInfo.depthAttachmentFormat = VulkanRHI::ToVkFormat(Desc.depth_format);
    RenderingCreateInfo.stencilAttachmentFormat = VulkanRHI::ToVkFormat(Desc.stencil_format);

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
    PipelineCreateInfo.layout = Device.GetBindlessDescriptorManager()->GetGlobalPipelineLayout();

    VkPipeline Pipeline = VK_NULL_HANDLE;
    VkResult Result = VulkanRHI::vkCreateGraphicsPipelines(Device.GetVkDevice(), DriverCache, 1, &PipelineCreateInfo, nullptr, &Pipeline);
    ensure(Result == VK_SUCCESS, "Failed to create graphics pipeline");

    return Pipeline;
}

uint64_t VulkanPipelineManager::Hash(const RHIPipelineDesc &P_Spec)
{
    uint64_t Hash = FNV_OFFSET;

    HashObject(Hash, P_Spec.vertex_shader_path);
    HashObject(Hash, P_Spec.pixel_shader_path);
    HashObject(Hash, P_Spec.depth_format);
    HashObject(Hash, P_Spec.stencil_format);
    HashObject(Hash, P_Spec.topology);
    HashObject(Hash, P_Spec.cull_mode);
    HashObject(Hash, P_Spec.polygon_mode);
    HashObject(Hash, P_Spec.depth_test_enable);
    HashObject(Hash, P_Spec.depth_write_enable);
    //HashBytes(Hash, P_Spec.ColorAttachmentFormats.data(), P_Spec.ColorAttachmentFormats.size() * sizeof(ERHIFormat));
    //HashBytes(Hash, P_Spec.VertexBindings.data(), P_Spec.VertexBindings.size() * sizeof(FRHIVertexBinding));
    //HashBytes(Hash, P_Spec.VertexAttributes.data(), P_Spec.VertexAttributes.size() * sizeof(FRHIVertexAttribute));

    return Hash;
}
