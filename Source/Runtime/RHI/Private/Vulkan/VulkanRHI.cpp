#include "VulkanRHI.h"

#include "Assert.h"
#include "FVulkanPipeline.h"
#include "Logger.h"
#include "MeshAsset.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBufferContext.h"
#include "VulkanCommandList.h"
#include "VulkanDescriptorManager.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanQueue.h"
#include "VulkanResourceManager.h"
#include "VulkanSampler.h"
#include "VulkanTexture.h"
#include "VulkanViewport.h"

#include <GLFW/glfw3.h>

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

    FrameCounter = 0;
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

void CVulkanRHI::EndFrame()
{
    Device->GetDescriptorManager()->Flush();

    Device->GetPresentQueue()->Submit();

    Viewport->Present();

    Device->GetPresentQueue()->Wait();

    FrameCounter++;

    Viewport->AcquireNextImageIndex();

    Device->GC();
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

uint64_t CVulkanRHI::GetFrameCounter() const noexcept
{
    return FrameCounter;
}

FRHITexture *CVulkanRHI::RHICreateTexture(FRHITextureDesc Desc) const
{
    return new FVulkanTexture(*Device, Desc);
}

FRHITextureView *CVulkanRHI::RHICreateTextureView(FRHITextureViewDesc TextureViewDesc, FRHITexture *Texture) const
{
    return new FVulkanTextureView(*Device, TextureViewDesc, static_cast<VkImage>(Texture->GetHandle()));
}

FRHIBuffer *CVulkanRHI::RHICreateBuffer(FRHIBufferDesc Desc) const
{
    return new FVulkanBuffer(*Device, Desc);
}

RRHISampler *CVulkanRHI::RHICreateSampler(RRHISamplerDesc Desc) const
{
    return new RVulkanSampler(*Device, Desc);
}

void CVulkanRHI::RHIUploadDataBlocking(FRHIBuffer *Buffer, void *Data, uint64_t Size)
{
    std::unique_ptr<FRHIBuffer> StagingBuffer(RHICreateBuffer(FRHIBufferDesc::CreateStagingBuffer(Size)));

    StagingBuffer->Map(Data, Size);

    Device->GetGraphicsQueue()->ImmediateSubmit([&](FRHICommandList& CmdList)
    {
        CmdList.Enqueue<FVulkanCommandCopyBuffer>(StagingBuffer.get(), Buffer, 0, 0, Size);
    });
}
void CVulkanRHI::RHIUploadTextureBlocking(FRHITexture *Texture2D, const void *Data, uint32_t Width, uint32_t Height)
{
    std::unique_ptr<FRHIBuffer> StagingBuffer(RHICreateBuffer(FRHIBufferDesc::CreateStagingBuffer(Width * Height * 4)));

    StagingBuffer->Map(Data, Width * Height * 4);

    Device->GetGraphicsQueue()->ImmediateSubmit([&](FRHICommandList& CmdList)
    {
        CmdList.Enqueue<FVulkanCommandPipelineBarrier>(Texture2D, ERHITextureLayout::Undefined, ERHITextureLayout::TransferDst, ERHIResourceAccess::None,
            ERHIResourceAccess::TransferWrite, ERHIPipelineStage::TopOfPipe, ERHIPipelineStage::Transfer);

        VkBufferImageCopy vkRegion = {};
        vkRegion.bufferOffset = 0;
        vkRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        vkRegion.imageSubresource.mipLevel       = 0;
        vkRegion.imageSubresource.baseArrayLayer = 0;
        vkRegion.imageSubresource.layerCount     = 1;
        vkRegion.imageOffset = {0, 0, 0};
        vkRegion.imageExtent = { (uint32_t)Width, (uint32_t)Height, 1 };

        CmdList.Enqueue<FVulkanCommandCopyBufferToImage>(StagingBuffer.get(), Texture2D, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vkRegion);

        CmdList.Enqueue<FVulkanCommandPipelineBarrier>(Texture2D, ERHITextureLayout::TransferDst, ERHITextureLayout::ShaderReadOnly, ERHIResourceAccess::TransferWrite, ERHIResourceAccess::ShaderSampledRead, ERHIPipelineStage::Transfer, ERHIPipelineStage::FragmentShader);
    });
}

FRHIPipeline *CVulkanRHI::RHICreatePipelineStateObject(FRHIPipelineDesc Desc)
{
    return Device->GetPipelineManager()->GetPipeline(Desc);
}

IRHICommandListContext *CVulkanRHI::GetCommandListContext() const
{
    return Device->GetCommandListContext();
}
void CVulkanRHI::RHICmdBindPipeline(FRHICommandList &RHICmdList, FRHIPipeline *Pipeline)
{
    RHICmdList.Enqueue<FVulkanCommandBindPipeline>(Pipeline);
}

void CVulkanRHI::RHICmdBeginRendering(FRHICommandList *RHICmdList, std::span<FRHITextureView*> ColorAttachments, FRHITextureView* DepthAttachmentView)
{
    RHICmdList->Enqueue<FVulkanCommandBeginRendering>(ColorAttachments, DepthAttachmentView);
}

void CVulkanRHI::RHICmdEndRendering(FRHICommandList *RHICmdList)
{
    RHICmdList->Enqueue<FVulkanCommandEndRendering>();
}

void CVulkanRHI::RHICmdPipelineBarrier(FRHICommandList *RHICmdList, FRHITexture *Texture, ERHITextureLayout SrcLayout, ERHITextureLayout DstLayout, ERHIResourceAccess SrcAccess, ERHIResourceAccess DstAccess, ERHIPipelineStage SrcStage, ERHIPipelineStage DstStage, uint32_t SrcQueueFamilyIndex, uint32_t DstQueueFamilyIndex)
{
    RHICmdList->Enqueue<FVulkanCommandPipelineBarrier>(Texture, SrcLayout, DstLayout, SrcAccess, DstAccess, SrcStage, DstStage, SrcQueueFamilyIndex, DstQueueFamilyIndex);
}

void CVulkanRHI::RHICmdDraw(FRHICommandList &RHICmdList, const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params)
{
    RHICmdList.Enqueue<FVulkanCommandDraw>(DrawCommand, Params);
}

void CVulkanRHI::RHICmdDrawIndexed(FRHICommandList& RHICmdList, const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params)
{
    RHICmdList.Enqueue<FVulkanCommandDrawIndexed>(DrawCommand, Params);
}

FRHITexture *CVulkanRHI::RHIGetBackBufferRenderTarget() const
{
    return Viewport->GetBackBufferTexture();
}

std::array<uint32_t, 2> CVulkanRHI::RHIGetViewportExtent() const
{
    return std::array<uint32_t, 2>{Viewport->GetExtent().width, Viewport->GetExtent().height};
}

void CVulkanRHI::RHICmdBlitImage(FRHICommandList &RHICmdList, FRHITexture *Src, FRHITexture *Dst)
{
    RHICmdList.Enqueue<FVulkanCommandBlitImage>(Src, Dst);
}

void CVulkanRHI::RHICmdSetScissor(FRHICommandList &RHICmdList, int32_t X, int32_t Y, uint32_t Width, uint32_t Height)
{
    RHICmdList.Enqueue<FVulkanCommandSetScissor>(X, Y, Width, Height);
}

std::shared_ptr<FRHIBuffer> CVulkanRHI::RHIBindBuffer(void *Data, uint64_t Size)
{
    std::shared_ptr<FRHIBuffer> Buffer(RHICreateBuffer(FRHIBufferDesc::CreateStorageBuffer(Size)));

    Buffer->Map(Data, Size);

    return Buffer;
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

std::span<const char *> CVulkanRHI::GetPhysicalDeviceExtensions()
{
    return std::span(PhysicalDeviceExtensions);
}

VkPipelineLayout CVulkanRHI::GetBindlessPipelineLayout() const
{
    return Device->GetDescriptorManager()->GetPipelineLayout();
}

VkDescriptorSet CVulkanRHI::GetBindlessDescriptorSet() const
{
    return Device->GetDescriptorManager()->GetDescriptorSet();
}

VkDescriptorSetLayout CVulkanRHI::GetBindlessDesciptorSetLayout() const
{
    return Device->GetDescriptorManager()->GetDescriptorSetLayout();
}

FVulkanDevice * CVulkanRHI::GetDevice() const
{
    assert(Device != nullptr);

    return Device;
}

FVulkanViewport* CVulkanRHI::GetViewport() const
{
    assert(Viewport != nullptr);

    return Viewport;
}
