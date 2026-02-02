#include "VulkanRHI.h"

#include "Assertion.h"
#include "Logger.h"
#include "Events/Delegate.h"
#include "Platform.h"
#include "VulkanPipeline.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorManager.h"
#include "VulkanQueue.h"
#include "VulkanTextureSampler.h"
#include "VulkanSwapchain.h"
#include "VulkanTexture.h"
#include "VulkanSurface.h"

#if RK_PLATFORM_WINDOWS
    #include <libloaderapi.h>
#endif
#include "VulkanTextureView.h"

static TDelegate<VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*> GDebugCallback;
static VKAPI_ATTR VkBool32 VKAPI_CALL ExecDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
{
    GDebugCallback.Broadcast(Severity, Type, CallbackData);
    return VK_FALSE;
}

CVulkanRHI::CVulkanRHI()
    : Instance(nullptr),
    Device(nullptr),
    FrameCounter(0)
{
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
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:   RK_LOG_DEBUG    ("[{}] {}", GetVulkanRHIMinimal()->GetName(), Data->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:      RK_LOG_INFO     ("[{}] {}", GetVulkanRHIMinimal()->GetName(), Data->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:   RK_LOG_WARN     ("[{}] {}", GetVulkanRHIMinimal()->GetName(), Data->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:     RK_LOG_ERROR    ("[{}] {}", GetVulkanRHIMinimal()->GetName(), Data->pMessage); break;
            default: break;
        }
    });

    std::vector<const char*> surfaceInstanceExtensions = Platform::GetSurfaceExtensions();
    InstanceExtensions.insert(InstanceExtensions.end(), surfaceInstanceExtensions.begin(), surfaceInstanceExtensions.end());
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
    ensure(Result == VK_SUCCESS, "Failed to initialize Vulkan instance.");

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
            Device = new VulkanDevice(CurrentPhysicalDevice);
            break;
        }
    }
}

void CVulkanRHI::Shutdown()
{
    RHIWaitUntilIdle();

    delete Device;

    VulkanRHI::vkDestroyInstance(Instance, nullptr);
}

void CVulkanRHI::Submit(RHICommandList& cmd_list)
{
    Device->GetGraphicsQueue()->Submit(cmd_list);
}

void CVulkanRHI::Reset()
{
    Device->GetGraphicsQueue()->Retire();

    Device->GC();

    FrameCounter++;

	Device->GetBindlessDescriptorManager()->BeginFrame();
}

const char* CVulkanRHI::GetName() const
{
    return "Vulkan";
}

const char* CVulkanRHI::GetVersion() const
{
    return nullptr;
}

RHIInterfaceType CVulkanRHI::GetInterfaceType() const noexcept
{
    return StaticType;
}

RHI* CVulkanRHI::GetNonValidationRHI() const noexcept
{
    return const_cast<CVulkanRHI*>(this);
}

void CVulkanRHI::RHIWaitUntilIdle() const
{
    Device->WaitUntilIdle();
}

uint64_t CVulkanRHI::RHIGetFrameCounter() const noexcept
{
    return FrameCounter;
}

RHISurface* CVulkanRHI::RHICreateSurface(const RHISurfaceDesc& Desc)
{
    return new VulkanSurface(*Device, Desc);
}

RHITexture *CVulkanRHI::RHICreateTexture(const RHITextureDesc& Desc) const
{
    return new VulkanTexture(*Device, Desc);
}

RHITextureView *CVulkanRHI::RHICreateTextureView(const RHITextureViewDesc& TextureViewDesc, RHITexture *Texture) const
{
    return new VulkanTextureView(*Device, TextureViewDesc, Texture);
}

RHIBuffer *CVulkanRHI::RHICreateBuffer(const RHIBufferDesc& Desc) const
{
    return new VulkanBuffer(*Device, Desc);
}

RHITextureSampler *CVulkanRHI::RHICreateTextureSampler(const RHITextureSamplerDesc& Desc) const
{
    return new VulkanTextureSampler(*Device, Desc);
}

RHIPipeline *CVulkanRHI::RHICreatePipeline(const RHIPipelineDesc& Desc)
{
    return Device->GetPipelineManager()->GetOrCreatePipeline(Desc);
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
    return GetDevice()->GetPersistentMemoryAllocator();
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

std::span<const char *> CVulkanRHI::GetValidationExtensions()
{
    return std::span(ValidationLayerExtensions);
}

std::span<const char*> CVulkanRHI::GetInstanceExtensions()
{
    return std::span(InstanceExtensions);
}

std::span<const char *> CVulkanRHI::GetPhysicalDeviceExtensions()
{
    return std::span(PhysicalDeviceExtensions);
}

VulkanDevice * CVulkanRHI::GetDevice() const
{
    ensure(Device != nullptr);

    return Device;
}