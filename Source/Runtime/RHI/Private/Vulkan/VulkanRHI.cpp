#include "VulkanRHI.h"

#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandList.h"
#include "VulkanQueue.h"
#include "VulkanDevice.h"
#include "VulkanViewport.h"
#include "VulkanCommandBufferContext.h"
#include "VulkanRenderGraph.h"
#include "Logger.h"
#include "Assert.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorManager.h"
#include "VulkanResourceManager.h"
#include "MeshAsset.h"
#include <GLFW/glfw3.h>


// TEMPORARY: Testing
#include <ShaderAsset.h>
AssetManager* m_AssetManager = new AssetManager();
AssetHandle<ShaderAsset> m_VertexShaderAsset;
AssetHandle<ShaderAsset> m_FragmentShaderAsset;
VulkanShader* m_VertexShader;
VulkanShader* m_FragmentShader;
BufferHandle m_VertexBuffer;
BufferHandle m_IndexBuffer;
BufferHandle m_StagingBuffer;
AssetHandle<MeshAsset> m_MeshAsset;

#if RK_PLATFORM_WINDOWS
#include <libloaderapi.h>
#endif

static TDelegate<VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*> GDebugCallback;
static VKAPI_ATTR VkBool32 VKAPI_CALL ExecDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
{
    GDebugCallback.Broadcast(Severity, Type, CallbackData);
    return VK_FALSE;
}

void CVulkanRHI::Init()
{
    bool bDebug = true;

#if RK_PLATFORM_WINDOWS
    HMODULE NTDLL = GetModuleHandleA("ntdll.dll");
    bDebug = !GetProcAddress(NTDLL, "wine_get_version");
#endif

    LoadVulkanLibrary();
    LoadVulkanGlobalFunc();

    GDebugCallback.Bind([](auto Severity, auto, const auto* Data)
    {
        switch(Severity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:   RK_LOG_DEBUG  ("[{}] {}", GetVulkanRHIMinimal()->GetName(), Data->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:      RK_LOG_INFO   ("[{}] {}", GetVulkanRHIMinimal()->GetName(), Data->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:   RK_LOG_WARNING("[{}] {}", GetVulkanRHIMinimal()->GetName(), Data->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:     RK_LOG_ERROR  ("[{}] {}", GetVulkanRHIMinimal()->GetName(), Data->pMessage); break;
            default: break;
        }
    });

    uint32_t GlfwExtensionCount = 0;
    const char** GlfwExtensions = glfwGetRequiredInstanceExtensions(&GlfwExtensionCount);
    InstanceExtensions.insert(InstanceExtensions.end(), GlfwExtensions, GlfwExtensions + GlfwExtensionCount);
    InstanceExtensions.push_back("VK_EXT_swapchain_colorspace");
    PhysicalDeviceExtensions.push_back("VK_KHR_swapchain");

    if (bDebug) 
    {
        InstanceExtensions.push_back("VK_EXT_debug_utils");
        ValidationLayerExtensions.push_back("VK_LAYER_KHRONOS_validation");
    }

    VkApplicationInfo ApplicationInfo = {};
    ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ApplicationInfo.pApplicationName = "Rocket";
    ApplicationInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    ApplicationInfo.pEngineName = "No Engine";
    ApplicationInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    ApplicationInfo.apiVersion = VK_API_VERSION_1_3;

    VkDebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo = {};
    DebugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    DebugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    DebugUtilsMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    DebugUtilsMessengerCreateInfo.pfnUserCallback = &ExecDebugCallback;

    VkInstanceCreateInfo InstanceCreateInfo = {};
    InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;
    InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());
    InstanceCreateInfo.ppEnabledExtensionNames = InstanceExtensions.data();
    InstanceCreateInfo.enabledLayerCount = ValidationLayerExtensions.size();
    InstanceCreateInfo.ppEnabledLayerNames = ValidationLayerExtensions.data();
    InstanceCreateInfo.pNext = &DebugUtilsMessengerCreateInfo;

    VkResult Result = VulkanRHI::vkCreateInstance(&InstanceCreateInfo, nullptr, &Instance);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to initialize Vulkan instance.");

    LoadVulkanInstanceFunc(RHIGetVkInstance());

    uint32_t DeviceCount = 0;
    VulkanRHI::vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);

    std::vector<VkPhysicalDevice> AllPhysicalDevices(DeviceCount);
    VulkanRHI::vkEnumeratePhysicalDevices(Instance, &DeviceCount, AllPhysicalDevices.data());

    std::vector<VkPhysicalDevice> DiscreteDevices;
    std::vector<VkPhysicalDevice> IntegratedDevices;
    std::vector<VkPhysicalDevice> VirtualDevices;

    for (VkPhysicalDevice PhysicalDevice : AllPhysicalDevices)
    {
        VkPhysicalDeviceProperties GPUProperties;
        VulkanRHI::vkGetPhysicalDeviceProperties(PhysicalDevice, &GPUProperties);

        if (GPUProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) DiscreteDevices.push_back(PhysicalDevice);
        if (GPUProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) IntegratedDevices.push_back(PhysicalDevice);
        if (GPUProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) VirtualDevices.push_back(PhysicalDevice);
    }

    std::vector<VkPhysicalDevice> AvailablePhysicalDevices;
    AvailablePhysicalDevices.insert(AvailablePhysicalDevices.end(), DiscreteDevices.begin(), DiscreteDevices.end());
    AvailablePhysicalDevices.insert(AvailablePhysicalDevices.end(), IntegratedDevices.begin(), IntegratedDevices.end());
    AvailablePhysicalDevices.insert(AvailablePhysicalDevices.end(), VirtualDevices.begin(), VirtualDevices.end());

    for (VkPhysicalDevice CurrentPhysicalDevice : AvailablePhysicalDevices)
    {
        uint32_t ExtensionCount;
        VulkanRHI::vkEnumerateDeviceExtensionProperties(CurrentPhysicalDevice, nullptr, &ExtensionCount, nullptr);

        std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
        VulkanRHI::vkEnumerateDeviceExtensionProperties(CurrentPhysicalDevice, nullptr, &ExtensionCount, AvailableExtensions.data());

        std::unordered_set<std::string_view> Filter;
        Filter.insert(PhysicalDeviceExtensions.begin(), PhysicalDeviceExtensions.end());

        for (VkExtensionProperties Extension : AvailableExtensions)
        {
            if (Filter.contains(Extension.extensionName))
            {
                Filter.erase(Extension.extensionName);
            }
        }

        if (Filter.empty())
        {
            // Found a suitable GPU
            Device = new FVulkanDevice(CurrentPhysicalDevice);
            break;
        }
    }

    Viewport = new FVulkanViewport(*Device);
    RenderGraph = new FVulkanRenderGraph(*Device);



    {
        m_VertexShaderAsset = m_AssetManager->Load<ShaderAsset>(AssetLoaderRequest { GAME_SRC "/Shader/VertexShader.hlsl" }, ShaderAssetOptions { "main", "vs_6_6" } );
        m_FragmentShaderAsset = m_AssetManager->Load<ShaderAsset>(AssetLoaderRequest { GAME_SRC "/Shader/FragmentShader.hlsl" }, ShaderAssetOptions { "main", "ps_6_6" } );

        auto VertexShaderAsset = m_AssetManager->Get(m_VertexShaderAsset);
        m_VertexShader = new VulkanShader(Device->GetVkDevice(), VK_SHADER_STAGE_VERTEX_BIT);
        m_VertexShader->Create(VertexShaderAsset->Data, VertexShaderAsset->WordCount);

        auto FragmentShaderAsset = m_AssetManager->Get(m_FragmentShaderAsset);
        m_FragmentShader = new VulkanShader(Device->GetVkDevice(), VK_SHADER_STAGE_FRAGMENT_BIT);
        m_FragmentShader->Create(FragmentShaderAsset->Data, FragmentShaderAsset->WordCount);

        BufferDesc VertexBufferDesc;
        VertexBufferDesc.Persistent = true;
        VertexBufferDesc.MemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        VertexBufferDesc.UsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VertexBufferDesc.Size = 64ull * 1024 * 1024;
        VertexBufferDesc.Persistent = true;
        m_VertexBuffer = Device->GetResourceManager()->AddBuffer(VertexBufferDesc);

        BufferDesc IndexBufferDesc;
        IndexBufferDesc.Persistent = true;
        IndexBufferDesc.MemoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        IndexBufferDesc.UsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        IndexBufferDesc.Size = 8ull * 1024 * 1024;
        IndexBufferDesc.Persistent = true;
        m_IndexBuffer = Device->GetResourceManager()->AddBuffer(IndexBufferDesc);

        BufferDesc StagingBufferDesc;
        StagingBufferDesc.Persistent = true;
        StagingBufferDesc.MemoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        StagingBufferDesc.UsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        StagingBufferDesc.Size = 256ull * 1024 * 1024;
        StagingBufferDesc.Persistent = true;
        m_StagingBuffer = Device->GetResourceManager()->AddBuffer(StagingBufferDesc);

        m_MeshAsset = m_AssetManager->Load<MeshAsset>(AssetLoaderRequest { GAME_CONTENT "/Cube_Sphere.glb" }, MeshAssetOptions {});

        Device->GetResourceManager()->MapBuffer(m_StagingBuffer, m_AssetManager->Get(m_MeshAsset)->Node.Primitives[0].Vertices.data(), m_AssetManager->Get(m_MeshAsset)->Node.Primitives[0].Vertices.size(), 0);
    }
}

void CVulkanRHI::Shutdown()
{
    RHIWaitUntilIdle();

    delete RenderGraph;
    delete Viewport;
    delete Device;

    RenderGraph = nullptr;
    Viewport = nullptr;
    Device = nullptr;

    VulkanRHI::vkDestroyInstance(Instance, nullptr);
}

void CVulkanRHI::Tick(float DeltaTime)
{
    GetDevice()->GetGraphicsQueue()->Await();

    Device->GetResourceManager()->GC();

    if (!Viewport->Acquire())
    {
        RK_LOG_ERROR("Failed to acquire a new swapchain image.");
        return;
    }

    RenderGraph->Reset();

    ImageDesc Desc;
    Desc.Extent = { 1024, 1024, 1 };
    Desc.Format = VK_FORMAT_B8G8R8A8_UNORM;
    Desc.UsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    Desc.AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    Desc.Persistent = false;
    Desc.MipLevels = 1;
    auto TextureHandle = Device->GetResourceManager()->AddImage(Desc);

    FVulkanRGBuilder Builder = RenderGraph->GetMutableBuilder();
    Builder.Import(TextureHandle);

    const FRGResourceAccess GBufferWrites[] = { WriteImage(TextureHandle) };
    const FRGResourceAccess PresentReads[] = { ReadImage(TextureHandle) };

    VkPipelineLayout PipelineLayout = Device->GetDescriptorManager()->GetPipelineLayout();
    VkDescriptorSet DescriptorSet = Device->GetDescriptorManager()->GetDescriptorSet();
    VulkanPipelineSpecification Spec = Device->GetPipelineManager()->MakeSpecification(PipelineLayout, m_VertexShader, m_FragmentShader);
    VkPipeline Pipeline = Device->GetPipelineManager()->GetPipeline(Spec);

    Builder.AddPass("Copy", EVulkanQueueType::Transfer, {}, {}, [&](FRHICommandList& RHICmdList)
    {
        //RHICmdList.Enqueue<VulkanCommandCopyBuffer>(Device->GetResourceManager()->GetBuffer(m_StagingBuffer), Device->GetResourceManager()->GetBuffer(m_VertexBuffer));
    });

    Builder.AddPass("GBuffer", EVulkanQueueType::Graphics, {}, GBufferWrites, [&](FRHICommandList& RHICmdList)
    {
        RHICmdList.Enqueue<FVulkanCommandSetMemoryBarrier>(GetViewport()->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        RHICmdList.Enqueue<FVulkanCommandBeginRendering>(GetViewport()->GetImageView());
        RHICmdList.Enqueue<VulkanCommandSetViewport>(GetViewport()->GetExtent().width, GetViewport()->GetExtent().height, 0.0f, 1.0f);
        RHICmdList.Enqueue<VulkanCommandSetScissor>(0, 0, GetViewport()->GetExtent().width, GetViewport()->GetExtent().height);
        RHICmdList.Enqueue<VulkanCommandBindPipeline>(VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
        RHICmdList.Enqueue<VulkanCommandBindDescriptorSets>(VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, 1, &DescriptorSet);
        //RHICmdList.Enqueue<VulkanCommandBindVertexBuffer>(); // incomplete
        //RHICmdList.Enqueue<VulkanCommandBindIndexBuffer>(); // incomplete
        RHICmdList.Enqueue<FVulkanCommandEndRendering>();
    });

    Builder.AddPass("Present", EVulkanQueueType::Graphics, PresentReads , {}, [&](FRHICommandList& RHICmdList)
    {
        RHICmdList.Enqueue<FVulkanCommandSetMemoryBarrier>(GetViewport()->GetImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    });

    RenderGraph->Compile();

    FVulkanCommandBufferPool* CommandBufferPool = GetDevice()->GetGraphicsQueue()->GetCommandBufferPool();
    FVulkanCommandBufferContext Ctx(*CommandBufferPool);

    Ctx.AddWaitSemaphore(Viewport->GetImageAcquiredSemaphore());
    RenderGraph->Execute(Ctx);
    Ctx.AddSignalSemaphore(Viewport->GetRenderFinishedSemaphore());

    Device->GetDescriptorManager()->Flush();

    GetDevice()->GetGraphicsQueue()->Submit(Ctx);

    Viewport->Present();
}

const char* CVulkanRHI::GetName() const
{
    return "Vulkan";
}

const char* CVulkanRHI::GetVersion() const
{
    return nullptr;
}

ERHIInterfaceType CVulkanRHI::GetInterfaceType() const noexcept
{
    return StaticType;
}

IRHI* CVulkanRHI::GetNonValidationRHI() const noexcept
{
    return const_cast<CVulkanRHI*>(this);
}

void CVulkanRHI::RHIWaitUntilIdle() const
{
    Device->WaitUntilIdle();
}

VkInstance CVulkanRHI::RHIGetVkInstance() const
{
    return Instance;
}

VkDevice CVulkanRHI::RHIGetVkDevice() const
{
    return GetDevice()->GetVkDevice();
}

VkPhysicalDevice CVulkanRHI::RHIGetVkPhysicalDevice() const
{
    return GetDevice()->GetVkPhysicalDevice();
}

VkQueue CVulkanRHI::RHIGetGraphicsVkQueue() const
{
    return GetDevice()->GetGraphicsQueue()->GetHandle();
}

VkQueue CVulkanRHI::RHIGetComputeVkQueue() const
{
    return GetDevice()->GetComputeQueue()->GetHandle();
}

VkQueue CVulkanRHI::RHIGetTransferVkQueue() const
{
    return GetDevice()->GetTransferQueue()->GetHandle();
}

VkQueue CVulkanRHI::RHIGetPresentVkQueue() const
{
    return GetDevice()->GetGraphicsQueue()->GetHandle();
}

uint32_t CVulkanRHI::RHIGetGraphicsQueueFamilyIndex() const
{
    return GetDevice()->GetGraphicsQueue()->GetQueueFamilyIndex();
}

uint32_t CVulkanRHI::RHIGetComputeQueueFamilyIndex() const
{
    return GetDevice()->GetComputeQueue()->GetQueueFamilyIndex();
}

uint32_t CVulkanRHI::RHIGetTransferQueueFamilyIndex() const
{
    return GetDevice()->GetTransferQueue()->GetQueueFamilyIndex();
}

uint32_t CVulkanRHI::RHIGetPresentQueueFamilyIndex() const
{
    return GetDevice()->GetGraphicsQueue()->GetQueueFamilyIndex();
}

VkSurfaceKHR CVulkanRHI::RHIGetVkSurface() const
{
    return Viewport->GetVkSurface();
}

VkSwapchainKHR CVulkanRHI::RHIGetVkSwapchain() const
{
    return Viewport->GetVkSwapchain();
}

VkExtent2D CVulkanRHI::RHIGetSwapchainExtent() const
{
    return Viewport->GetExtent();
}

VkPresentModeKHR CVulkanRHI::RHIGetSwapchainPresentMode() const
{
    return Viewport->GetPresentMode();
}

VkSurfaceFormatKHR CVulkanRHI::RHIGetSwapchainSurfaceFormat() const
{
    return Viewport->GetSurfaceFormat();
}

std::span<const char *> CVulkanRHI::GetValidationExtensions()
{
    return std::span(ValidationLayerExtensions);
}

std::span<const char*> CVulkanRHI::GetInstanceExtensions()
{
    return std::span(InstanceExtensions);
}

std::span<const char*> CVulkanRHI::GetPhysicalDeviceExtensions()
{
    return std::span(PhysicalDeviceExtensions);
}

FVulkanDevice* CVulkanRHI::GetDevice() const
{
    return Device;
}

FVulkanViewport* CVulkanRHI::GetViewport() const
{
    return Viewport;
}
