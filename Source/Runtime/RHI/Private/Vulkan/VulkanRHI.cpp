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
#include "VulkanSampler.h"
#include "VulkanSwapchain.h"
#include "VulkanTexture.h"
#include "VulkanSurface.h"

#if RK_PLATFORM_WINDOWS
    #include <libloaderapi.h>
#endif

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
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:   RK_LOG_DEBUG    ("[{}] {}", GetVulkanRHIMinimal()->get_name(), Data->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:      RK_LOG_INFO     ("[{}] {}", GetVulkanRHIMinimal()->get_name(), Data->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:   RK_LOG_WARN     ("[{}] {}", GetVulkanRHIMinimal()->get_name(), Data->pMessage); break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:     RK_LOG_ERROR    ("[{}] {}", GetVulkanRHIMinimal()->get_name(), Data->pMessage); break;
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

    FrameCounter = 0;
}

void CVulkanRHI::Shutdown()
{
    rhi_wait_until_idle();

    //for (auto Viewport : Viewports)
    //{
    //    delete Viewport;
    //}

    delete Device;
    Device = nullptr;

    VulkanRHI::vkDestroyInstance(Instance, nullptr);
}

//void CVulkanRHI::BeginFrame()
//{
////    FFrameGraph Graph;
////
////    auto Builder = Graph.Builder();
////
////    RRHIViewport* Viewport = Viewports[0];
////    Viewport->Acquire();
////    Graph.AddViewportSync(Viewport);
////
////    FRHITexture* BackBuffer = Viewport->GetBackBufferTexture();
////
////    FRGTextureState BackBufferInitial = { BackBuffer->CurrentLayout, BackBuffer->CurrentAccess, BackBuffer->CurrentPipelineStage };
////    FRGTextureState BackBufferFinal = { ERHITextureLayout::Present, ERHIResourceAccess::None, ERHIPipelineStage::BottomOfPipe };
////
////    FRGTextureHandle RGBackBuffer = Builder.RegisterExternalTexture("BackBuffers", BackBuffer, nullptr, BackBufferInitial, BackBufferFinal);
////
////    FRHITextureViewDesc BackBufferViewDesc = FRHITextureViewDesc::Create2D(BackBuffer->RHIGetTextureDesc().Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient);
////    FRGTextureViewHandle RGBackBufferView = Builder.CreateTextureView("BackBufferView", BackBufferViewDesc, RGBackBuffer);
////
////    const uint32_t W = BackBuffer->RHIGetTextureDesc().Extent[0];
////    const uint32_t H = BackBuffer->RHIGetTextureDesc().Extent[1];
////
////    FRHITextureDesc AlbedoDesc = FRHITextureDesc::Create2D(W, H, ERHIResourceLifetime::Transient, ERHITextureFormat::R8G8B8A8_UNORM, ERHITextureUsage::Color | ERHITextureUsage::SRV, ERHITextureLayout::Undefined, 1, 1, 1);
////    FRHITextureDesc NormalDesc = FRHITextureDesc::Create2D(W, H, ERHIResourceLifetime::Transient, ERHITextureFormat::R8G8B8A8_UNORM, ERHITextureUsage::Color | ERHITextureUsage::SRV, ERHITextureLayout::Undefined, 1, 1, 1);
////    FRHITextureDesc DepthDesc = FRHITextureDesc::Create2D(W, H, ERHIResourceLifetime::Transient, ERHITextureFormat::D32_SFLOAT, ERHITextureUsage::Depth | ERHITextureUsage::SRV, ERHITextureLayout::Undefined, 1, 1, 1);
////
////    FRGTextureHandle RGAlbedo = Builder.CreateTexture("GBuffer.Albedo", AlbedoDesc);
////    FRGTextureHandle RGNormal = Builder.CreateTexture("GBuffer.Normal", NormalDesc);
////    FRGTextureHandle RGDepth = Builder.CreateTexture("GBuffer.Depth", DepthDesc);
////
////    FRGTextureViewHandle RGAlbedoRTV = Builder.CreateTextureView("GBuffer.Albedo.RTV", FRHITextureViewDesc::Create2D(AlbedoDesc.Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), RGAlbedo);
////    FRGTextureViewHandle RGNormalRTV = Builder.CreateTextureView("GBuffer.Normal.RTV", FRHITextureViewDesc::Create2D(NormalDesc.Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), RGNormal);
////    FRGTextureViewHandle RGDepthDSV = Builder.CreateTextureView("GBuffer.Depth.DSV", FRHITextureViewDesc::Create2D(DepthDesc.Format, ERHITextureAspect::Depth, ERHIResourceLifetime::Transient), RGDepth);
////
////    FRGTextureViewHandle RGAlbedoSRV = Builder.CreateTextureView("GBuffer.Albedo.SRV", FRHITextureViewDesc::Create2D(AlbedoDesc.Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), RGAlbedo);
////    FRGTextureViewHandle RGNormalSRV = Builder.CreateTextureView("GBuffer.Normal.SRV", FRHITextureViewDesc::Create2D(NormalDesc.Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), RGNormal);
////    FRGTextureViewHandle RGDepthSRV = Builder.CreateTextureView("GBuffer.Depth.SRV", FRHITextureViewDesc::Create2D(DepthDesc.Format, ERHITextureAspect::Depth, ERHIResourceLifetime::Transient), RGDepth);
////
////    FRHITextureDesc HDRDesc = FRHITextureDesc::Create2D(W, H, ERHIResourceLifetime::Transient, ERHITextureFormat::R8G8B8A8_UNORM, ERHITextureUsage::Color | ERHITextureUsage::SRV, ERHITextureLayout::Undefined, 1, 1, 1);
////    FRGTextureHandle RGHDR = Builder.CreateTexture("Lighting.HDR", HDRDesc);
////
////    FRGTextureViewHandle RGHDRRTV = Builder.CreateTextureView("Lighting.HDR.RTV", FRHITextureViewDesc::Create2D(HDRDesc.Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), RGHDR);
////    FRGTextureViewHandle RGHDRSRV = Builder.CreateTextureView("Lighting.HDR.SRV", FRHITextureViewDesc::Create2D(HDRDesc.Format, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), RGHDR);
////
////    FRHIBufferDesc FrameCBDesc = {};
////    FrameCBDesc.Size = 256;
////    FrameCBDesc.Lifetime = ERHIResourceLifetime::Transient;
////    FrameCBDesc.Memory = ERHIResourceMemory::HostVisible;
////    FrameCBDesc.Usage = ERHIBufferUsage::Storage | ERHIBufferUsage::TransferDst;
////
////    FRGBufferHandle RGFrameCB = Builder.CreateBuffer("FrameCB", FrameCBDesc);
////
////    FRHIBufferDesc IndirectDesc = {};
////    IndirectDesc.Size = 1024;
////    IndirectDesc.Lifetime = ERHIResourceLifetime::Transient;
////    IndirectDesc.Memory = ERHIResourceMemory::HostVisible;
////    IndirectDesc.Usage = ERHIBufferUsage::Storage | ERHIBufferUsage::Indirect;
////
////    FRGBufferHandle RGIndirectArgs = Builder.CreateBuffer("IndirectArgs", IndirectDesc);
////
////    Builder.AddPass("UploadFrameConstants",
////        [&](FFrameGraphBuilder& B, FRGPass& Pass)
////        {
////            B.WriteBuffer(Pass, RGFrameCB, ERGBufferUsage::CopyDst);
////        },
////        [&](IRHICommandListContext& CmdListCtx)
////        {
////            FRHIBuffer* FrameCB = Builder.ResolveBuffer(RGFrameCB);
////
////            struct FFrameConstants { /* viewProj, cameraPos, etc */ uint8_t Data[256]; } Constants = {};
////
////            FrameCB->Map(&Constants, sizeof(Constants), 0);
////        });
////
////    Builder.AddPass("CullCompute",
////        [&](FFrameGraphBuilder& B, FRGPass& Pass)
////        {
////            B.ReadBuffer(Pass, RGFrameCB, ERGBufferUsage::StorageRead);     // for compute visibility/cull
////            B.WriteBuffer(Pass, RGIndirectArgs, ERGBufferUsage::StorageWrite);
////        },
////        [&](IRHICommandListContext& CmdListCtx)
////        {
////            // Bind compute pipeline, bind FrameCB + IndirectArgs (as storage), dispatch...
////            // CmdListCtx.RHISetPipeline(CullPipeline);
////            // CmdListCtx.RHIBindStorageBuffer(0, Builder.ResolveBuffer(RGFrameCB));
////            // CmdListCtx.RHIBindStorageBuffer(1, Builder.ResolveBuffer(RGIndirectArgs));
////            // CmdListCtx.RHIDispatch(...);
////        });
////
////    Builder.AddPass("GBuffer",
////        [&](FFrameGraphBuilder& B, FRGPass& Pass)
////        {
////            B.WriteTexture(Pass, RGAlbedo, ERGTextureUsage::ColorAttachment);
////            B.WriteTexture(Pass, RGNormal, ERGTextureUsage::ColorAttachment);
////            B.WriteTexture(Pass, RGDepth, ERGTextureUsage::DepthStencilAttachment);
////
////            B.ReadBuffer(Pass, RGFrameCB, ERGBufferUsage::Uniform);
////            B.ReadBuffer(Pass, RGIndirectArgs, ERGBufferUsage::Indirect);
////        },
////        [&](IRHICommandListContext& CmdListCtx)
////        {
////            FRHITextureView* AlbedoRTV = Builder.ResolveTextureView(RGAlbedoRTV);
////            FRHITextureView* NormalRTV = Builder.ResolveTextureView(RGNormalRTV);
////            FRHITextureView* DepthDSV = Builder.ResolveTextureView(RGDepthDSV);
////
////            std::array<FRHITextureView*, 2> Colors = { AlbedoRTV, NormalRTV };
////            CmdListCtx.RHIBeginRendering(Colors, DepthDSV, /*bClear*/true);
////
////            // Bind graphics pipeline, bind FrameCB, then draw using indirect args...
////            // CmdListCtx.RHISetPipeline(GBufferPipeline);
////            // CmdListCtx.RHIBindUniformBuffer(0, Builder.ResolveBuffer(RGFrameCB));
////            // CmdListCtx.RHIDrawIndexedIndirect(Builder.ResolveBuffer(RGIndirectArgs), ...);
////
////            CmdListCtx.RHIEndRendering();
////        });
////
////    Builder.AddPass("Lighting",
////        [&](FFrameGraphBuilder& B, FRGPass& Pass)
////        {
////            B.ReadTexture(Pass, RGAlbedo, ERGTextureUsage::ShaderRead);
////            B.ReadTexture(Pass, RGNormal, ERGTextureUsage::ShaderRead);
////            B.ReadTexture(Pass, RGDepth, ERGTextureUsage::ShaderRead);
////
////            B.ReadBuffer(Pass, RGFrameCB, ERGBufferUsage::Uniform);
////
////            B.WriteTexture(Pass, RGHDR, ERGTextureUsage::ColorAttachment);
////        },
////        [&](IRHICommandListContext& CmdListCtx)
////        {
////            FRHITextureView* HDRRTV = Builder.ResolveTextureView(RGHDRRTV);
////            std::array<FRHITextureView*, 1> Colors = { HDRRTV };
////
////            CmdListCtx.RHIBeginRendering(Colors, nullptr, /*bClear*/true);
////
////            // Bind lighting pipeline, bind SRVs for Albedo/Normal/Depth, draw fullscreen triangle...
////            // CmdListCtx.RHISetPipeline(LightingPipeline);
////            // CmdListCtx.RHIBindSampledTexture(0, Builder.ResolveTextureView(RGAlbedoSRV));
////            // CmdListCtx.RHIBindSampledTexture(1, Builder.ResolveTextureView(RGNormalSRV));
////            // CmdListCtx.RHIBindSampledTexture(2, Builder.ResolveTextureView(RGDepthSRV));
////            // CmdListCtx.RHIDraw(3, 1, 0, 0);
////
////            CmdListCtx.RHIEndRendering();
////        });
////
////    Builder.AddPass("Tonemap",
////        [&](FFrameGraphBuilder& B, FRGPass& Pass)
////        {
////            B.ReadTexture(Pass, RGHDR, ERGTextureUsage::ShaderRead);
////            B.WriteTexture(Pass, RGBackBuffer, ERGTextureUsage::ColorAttachment);
////        },
////        [&](IRHICommandListContext& CmdListCtx)
////        {
////            FRHITextureView* BackBufferView = Builder.ResolveTextureView(RGBackBufferView);
////
////            std::array<FRHITextureView*, 1> Colors = { BackBufferView };
////            CmdListCtx.RHIBeginRendering(Colors, nullptr, /*bClear*/false);
////
////            // Bind tonemap pipeline, bind HDR SRV, draw fullscreen triangle...
////            // CmdListCtx.RHISetPipeline(TonemapPipeline);
////            // CmdListCtx.RHIBindSampledTexture(0, Builder.ResolveTextureView(RGHDRSRV));
////            // CmdListCtx.RHIDraw(3, 1, 0, 0);
////
////            CmdListCtx.RHIEndRendering();
////        });
////
////    Builder.AddPass("BackBufferToPresent",
////        [&](FFrameGraphBuilder& B, FRGPass& Pass)
////        {
////            B.ReadTexture(Pass, RGBackBuffer, ERGTextureUsage::Present);
////        },
////        [&](IRHICommandListContext&)
////        {
////        });
////
////    FFrameDependencyGraph FDG = Graph.Compile();
////
////#if 1
////    Graph.Dump(FDG);
////#endif
////
////    Graph.Execute(*Device->GetCommandListContext());
//
//    if (!Device->GetCommandListContext()->ShouldFlush())
//        return;
//
//    Device->GetDescriptorManager()->Update();
//    Device->GetPresentQueue()->Submit();
//
//    //Graph.Reset();
//    //Viewports[0]->Present();
//}

void CVulkanRHI::Submit(RHICommandList& cmd_list)
{
    Device->GetPresentQueue()->Submit(cmd_list);
}

void CVulkanRHI::Reset()
{
    Device->GetPresentQueue()->Wait();

    FrameCounter++;

    Device->GC();

	Device->GetBindlessDescriptorManager()->BeginFrame();
}

const char* CVulkanRHI::get_name() const
{
    return "Vulkan";
}

const char* CVulkanRHI::get_version() const
{
    return nullptr;
}

RHIInterfaceType CVulkanRHI::get_interface_type() const noexcept
{
    return StaticType;
}

RHI* CVulkanRHI::get_non_validation_rhi() const noexcept
{
    return const_cast<CVulkanRHI*>(this);
}

void CVulkanRHI::rhi_wait_until_idle() const
{
    Device->WaitUntilIdle();
}

uint64_t CVulkanRHI::rhi_get_frame_counter() const noexcept
{
    return FrameCounter;
}

RHISurface* CVulkanRHI::CreateSurface(RHISurfaceDesc desc)
{
    return new VulkanSurface(*Device, desc);
}

RHITexture *CVulkanRHI::RHICreateTexture(RHITextureDesc Desc) const
{
    return new VulkanTexture(*Device, Desc);
}

RHITextureView *CVulkanRHI::RHICreateTextureView(RHITextureViewDesc TextureViewDesc, RHITexture *Texture) const
{
    return new VulkanTextureView(*Device, TextureViewDesc, Texture);
}

RHIBuffer *CVulkanRHI::RHICreateBuffer(RHIBufferDesc Desc) const
{
    return new VulkanBuffer(*Device, Desc);
}

RHISampler *CVulkanRHI::RHICreateSampler(RHISamplerDesc Desc) const
{
    return new VulkanSampler(*Device, Desc);
}

RHIPipeline *CVulkanRHI::RHICreatePipeline(RHIPipelineDesc Desc)
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
    return GetDevice()->get_vma_allocator();
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