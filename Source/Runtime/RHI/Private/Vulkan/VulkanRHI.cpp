#include "VulkanRHI.h"

#include "Assertion.h"
#include "Logger.h"
#include "Events/Delegate.h"
#include "Platform.h"
#include "VulkanPipeline.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanBuffer.h"
#include "VulkanCommandList.h"
#include "VulkanDescriptorManager.h"
#include "VulkanQueue.h"
#include "VulkanResourceManager.h"
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
            Device = new FVulkanDevice(CurrentPhysicalDevice);
            break;
        }
    }

    FrameCounter = 0;
}

void CVulkanRHI::Shutdown()
{
    RHIWaitUntilIdle();

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

void CVulkanRHI::submit()
{
    Device->GetDescriptorManager()->Update();

    Device->GetPresentQueue()->Submit();
}

void CVulkanRHI::reset()
{
    Device->GetPresentQueue()->Wait();

    FrameCounter++;

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

RHISurface* CVulkanRHI::create_surface(RHISurfaceDesc& desc)
{
    return new VulkanSurface(*Device, desc);
}

void CVulkanRHI::RHIWaitUntilIdle() const
{
    Device->WaitUntilIdle();
}

uint64_t CVulkanRHI::GetFrameCounter() const noexcept
{
    return FrameCounter;
}

RHITexture *CVulkanRHI::RHICreateTexture(FRHITextureDesc Desc) const
{
    return new FVulkanTexture(*Device, Desc);
}

FRHITextureView *CVulkanRHI::RHICreateTextureView(FRHITextureViewDesc TextureViewDesc, RHITexture *Texture) const
{
    return new FVulkanTextureView(*Device, TextureViewDesc, Texture);
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

    Device->GetGraphicsQueue()->ImmediateSubmit([&](RHICommandList& CmdList)
    {
        CmdList.Enqueue<FVulkanCommandCopyBuffer>(StagingBuffer.get(), Buffer, 0, 0, Size);
    });
}

void CVulkanRHI::RHIUploadTextureBlocking(RHITexture *Texture2D, const uint8_t *Data, uint32_t Width, uint32_t Height)
{
    std::unique_ptr<FRHIBuffer> StagingBuffer(RHICreateBuffer(FRHIBufferDesc::CreateStagingBuffer(Width * Height * GetTextureFormatPixelByteSize(Texture2D->RHIGetTextureDesc().Format))));

    StagingBuffer->Map(Data, Width * Height * GetTextureFormatPixelByteSize(Texture2D->RHIGetTextureDesc().Format));

    Device->GetGraphicsQueue()->ImmediateSubmit([&](RHICommandList& CmdList)
    {
        CmdList.Enqueue<FVulkanCommandPipelineBarrier>(Texture2D, ERHITextureLayout::Undefined, ERHITextureLayout::TransferDst, ERHIResourceAccess::None, ERHIResourceAccess::TransferWrite, ERHIPipelineStage::TopOfPipe, ERHIPipelineStage::Transfer);
        CmdList.Enqueue<FVulkanCommandCopyBufferToImage>(StagingBuffer.get(), Texture2D, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Width, Height);
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

void CVulkanRHI::RHICmdBindPipeline(RHICommandList &RHICmdList, FRHIPipeline *Pipeline)
{
    RHICmdList.Enqueue<FVulkanCommandBindPipeline>(Pipeline);
}

void CVulkanRHI::RHICmdBeginRendering(RHICommandList *RHICmdList, std::span<FRHITextureView*> ColorAttachments, FRHITextureView* DepthAttachmentView, bool bClearColor)
{
    RHICmdList->Enqueue<FVulkanCommandBeginRendering>(ColorAttachments, DepthAttachmentView, bClearColor);
}

void CVulkanRHI::RHICmdEndRendering(RHICommandList *RHICmdList)
{
    RHICmdList->Enqueue<FVulkanCommandEndRendering>();
}

void CVulkanRHI::RHICmdPipelineBarrier(RHICommandList *RHICmdList, RHITexture *Texture, ERHITextureLayout SrcLayout, ERHITextureLayout DstLayout, ERHIResourceAccess SrcAccess, ERHIResourceAccess DstAccess, ERHIPipelineStage SrcStage, ERHIPipelineStage DstStage, uint32_t SrcQueueFamilyIndex, uint32_t DstQueueFamilyIndex)
{
    RHICmdList->Enqueue<FVulkanCommandPipelineBarrier>(Texture, SrcLayout, DstLayout, SrcAccess, DstAccess, SrcStage, DstStage, SrcQueueFamilyIndex, DstQueueFamilyIndex);
}

void CVulkanRHI::RHICmdDraw(RHICommandList &RHICmdList, const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params)
{
    RHICmdList.Enqueue<FVulkanCommandDraw>(DrawCommand, Params);
}

void CVulkanRHI::RHICmdDrawIndexed(RHICommandList& RHICmdList, const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params)
{
}

void CVulkanRHI::RHICmdDrawIndexed2(RHICommandList& RHICmdList, const FRHIDrawCall& DrawCall)
{
    RHICmdList.Enqueue<FVulkanCommandDrawIndexed2>(DrawCall);
}

std::array<uint32_t, 2> CVulkanRHI::RHIGetViewportExtent() const
{
    ensure(false, "Not yet implemented.");
    return { 0, 0 };
}

void CVulkanRHI::RHICmdBlitImage(RHICommandList &RHICmdList, RHITexture *Src, RHITexture *Dst)
{
    RHICmdList.Enqueue<FVulkanCommandBlitImage>(Src, Dst);
}

void CVulkanRHI::RHICmdSetScissor(RHICommandList &RHICmdList, int32_t X, int32_t Y, uint32_t Width, uint32_t Height)
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
    ensure(Device != nullptr);

    return Device;
}