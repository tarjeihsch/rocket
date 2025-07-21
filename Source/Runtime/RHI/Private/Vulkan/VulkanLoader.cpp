#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanLoader.h"

#include <assert.h>

#ifdef RK_PLATFORM_WINDOWS
    #include <windows.h>
    #define VK_DLSYM(lib, name) GetProcAddress((HMODULE)(lib), name)
#endif

#ifdef RK_PLATFORM_LINUX
    #include <dlfcn.h>
    #define VK_DLSYM(lib, name) dlsym(lib, name)
#endif

namespace VulkanRHI
{
    void* Library = nullptr;

    #define DEFINE_PFN(name) PFN_##name name = nullptr;
    VULKAN_ALL_FUNCTIONS(DEFINE_PFN)
    #undef DEFINE_PFN
};

void LoadVulkanLibrary()
{
#if RK_PLATFORM_WINDOWS
    VulkanRHI::Library = LoadLibraryA("vulkan-1.dll");
    VulkanRHI::vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(static_cast<HMODULE>(VulkanRHI::Library), "vkGetInstanceProcAddr"));
#endif

#if RK_PLATFORM_LINUX
    VulkanRHI::Library = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    VulkanRHI::vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(VulkanRHI::Library, "vkGetInstanceProcAddr"));
#endif

    assert(VulkanRHI::Library && "Unable to find and load Vulkan dynamic library.");
}

void LoadVulkanGlobalFunc()
{
    #define LOAD_GLOBAL_FN(Name) VulkanRHI::Name = reinterpret_cast<PFN_##Name>(VulkanRHI::vkGetInstanceProcAddr(nullptr, #Name));
    VULKAN_GLOBAL_FUNCTIONS(LOAD_GLOBAL_FN)
    #undef LOAD_GLOBAL_FN
}

void LoadVulkanInstanceFunc(VkInstance Instance)
{
    assert(VulkanRHI::vkGetInstanceProcAddr);

    #define LOAD_INSTANCE_FN(Name) VulkanRHI::Name = reinterpret_cast<PFN_##Name>(VulkanRHI::vkGetInstanceProcAddr(Instance, #Name));
    VULKAN_INSTANCE_FUNCTIONS(LOAD_INSTANCE_FN);
    #undef LOAD_INSTANCE_FN
}

void LoadVulkanDeviceFunc(VkDevice Device)
{
    assert(VulkanRHI::vkGetDeviceProcAddr);

    #define LOAD_DEVICE_FN(Name) VulkanRHI::Name = reinterpret_cast<PFN_##Name>(VulkanRHI::vkGetDeviceProcAddr(Device, #Name));
    VULKAN_DEVICE_FUNCTIONS(LOAD_DEVICE_FN);
    #undef LOAD_DEVICE_FN
}

VmaVulkanFunctions CreateVmaFuncTable()
{
    VmaVulkanFunctions Funcs = {};

    #define ASSIGN_VMA(name) Funcs.name = VulkanRHI::name;
    VMA_BASE_FUNCTIONS(ASSIGN_VMA)
    #undef ASSIGN_VMA

    Funcs.vkGetBufferMemoryRequirements2KHR = VulkanRHI::vkGetBufferMemoryRequirements2;
    Funcs.vkGetImageMemoryRequirements2KHR = VulkanRHI::vkGetImageMemoryRequirements2;
    Funcs.vkBindBufferMemory2KHR = VulkanRHI::vkBindBufferMemory2;
    Funcs.vkBindImageMemory2KHR = VulkanRHI::vkBindImageMemory2;
    Funcs.vkGetPhysicalDeviceMemoryProperties2KHR = VulkanRHI::vkGetPhysicalDeviceMemoryProperties2;
    Funcs.vkGetDeviceBufferMemoryRequirements = VulkanRHI::vkGetDeviceBufferMemoryRequirements;
    Funcs.vkGetDeviceImageMemoryRequirements = VulkanRHI::vkGetDeviceImageMemoryRequirements;

    return Funcs;
}

void FreeVulkanLibrary()
{
#if RK_PLATFORM_LINUX
    dlclose(VulkanRHI::Library);
    VulkanRHI::Library = nullptr;
#endif
}
