#include "VulkanRHI.h"

#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandList.h"
#include "VulkanQueue.h"
#include "VulkanDevice.h"
#include "VulkanViewport.h"
#include "VulkanCommandBufferContext.h"
#include "VulkanFrameGraph.h"
#include "Logger.h"
#include "Assert.h"
#include "VulkanPipeline.h"
#include "VulkanDescriptorManager.h"
#include "VulkanResourceManager.h"
#include "MeshAsset.h"
#include <GLFW/glfw3.h>


// TEMPORARY: Testing
#include "VulkanTexture.h"

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
}

void CVulkanRHI::Shutdown()
{
    RHIWaitUntilIdle();

    delete Viewport;
    delete Device;

    Viewport = nullptr;
    Device = nullptr;

    VulkanRHI::vkDestroyInstance(Instance, nullptr);
}

// todo: Store FrameGraph as a FRAME object
// use FrameGraphBuilder to create passes at different places in code.
// Add each framegraphbuilder to a vector and loop through all the builds at FrameGraph::Execute.
// This way, we can add a framegraphbuilder that is for Graphics only, or for Present only.

void CVulkanRHI::Tick(float DeltaTime)
{
    GetDevice()->GetGraphicsQueue()->Await();

    Device->GC();

    if (!Viewport->Acquire())
    {
        RK_LOG_ERROR("Failed to acquire a new swapchain image.");
        return;
    }

    FFrameGraph FrameGraph(*Device);

    FRHITextureDesc RenderTargetDesc = FRHITextureDesc::Create2D(VkExtent3D(1280, 720, 1), VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, ERHITextureFlags::Transient);
    FRHITextureViewDesc RenderTargetViewDesc = FRHITextureViewDesc::Create2D(VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    FFGTextureRef BackBufferRef = FrameGraph.ImportTexture(Viewport->GetBackBufferTexture());
    FFGTextureRef RenderTargetRef = FrameGraph.CreateTexture(RenderTargetDesc, RenderTargetViewDesc);

    FFGPass& DrawPass = FrameGraph.AddPass("Draw", nullptr);
    DrawPass.Callback = [&](FRHICommandList& CmdList)
    {

    };

    FFGPass& PresentPass = FrameGraph.AddPass("Present", nullptr);
    PresentPass.AddPresentSrc(&BackBufferRef);

    FrameGraph.Execute();

    Device->GetDescriptorManager()->Flush();

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

FRHITexture * CVulkanRHI::RHICreateTexture(FRHITextureDesc &Desc) const
{
    return nullptr;
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

VmaAllocator CVulkanRHI::RHIGetVmaAllocator() const
{
    return GetDevice()->GetVmaAllocator();
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

VkSemaphore CVulkanRHI::GetImageAcquiredSemaphore() const
{
    return Viewport->GetImageAcquiredSemaphore();
}

VkSemaphore CVulkanRHI::GetRenderFinishedSemaphore() const
{
    return Viewport->GetRenderFinishedSemaphore();
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
    assert(Device != nullptr);

    return Device;
}

FVulkanViewport* CVulkanRHI::GetViewport() const
{
    assert(Viewport != nullptr);

    return Viewport;
}
