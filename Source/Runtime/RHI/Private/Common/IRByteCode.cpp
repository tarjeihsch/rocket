#include "IRByteCode.h"

#include "Assertion.h"

#ifdef RK_PLATFORM_LINUX
    #include <dlfcn.h>
    #include <dxc/dxcapi.h>
#endif

#ifdef RK_PLATFORM_WINDOWS
    #include <windows.h>
    #include <dxcapi.h>
    #include <wrl/client.h>
    using Microsoft::WRL::ComPtr;
#endif

IRByteCode AssetLoader<IRByteCode, IRByteCodeLoaderOptions>::Load(const AssetLoaderRequest& loaderRequest, const IRByteCodeLoaderOptions& loaderOptions)
{
    IRByteCode Result;

#ifdef RK_PLATFORM_LINUX
    void* DxcLibHandle = dlopen(ROCKET_BINARIES "/ThirdParty/Vulkan/lib/libdxcompiler.so", RTLD_LAZY);
    ensure(DxcLibHandle, "Failed to dynamically load DXC library (Ensure DXC is installed and environment is set).");

    using DxcCreateInstanceProc = HRESULT(__stdcall*)(const IID&, const IID&, void**);
    auto DxcCreateInstance = reinterpret_cast<DxcCreateInstanceProc>(dlsym(DxcLibHandle, "DxcCreateInstance"));
    ensure(DxcCreateInstance, "Failed to locate DxcCreateInstance function address.");

    CComPtr<IDxcBlob> ShaderBlob;
    CComPtr<IDxcCompiler> Compiler;
    CComPtr<IDxcLibrary> Library;
    CComPtr<IDxcIncludeHandler> IncludeHandler;
    CComPtr<IDxcBlobEncoding> SourceBlob;
    CComPtr<IDxcOperationResult> OperationResult;

    // Create the DXC library instance
    HRESULT Result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler));
    ensure(!FAILED(Result), "Failed to create DXC Library instance.");

    // Create the DXC compiler instance
    Result = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&Library));
    ensure(!FAILED(Result), "Failed to create DXC Compiler instance.");

    // Load and encode the shader source file
    Result = Library->CreateBlobFromFile(std::wstring(loaderRequest.Path.begin(), loaderRequest.Path.end()).c_str(), nullptr, &SourceBlob);
    ensure(!FAILED(Result), "Failed to load shader source file.");

    std::wstring TargetProfileW(loaderOptions.TargetProfile.begin(), loaderOptions.TargetProfile.end());
    std::vector<LPCWSTR> Arguments;
    Arguments.push_back(L"-T");
    Arguments.push_back(TargetProfileW.c_str());
    Arguments.push_back(L"-spirv");
    Arguments.push_back(L"-fspv-target-env=vulkan1.3");
    Arguments.push_back(L"-fspv-extension=SPV_KHR_physical_storage_buffer");
    Arguments.push_back(L"-fspv-extension=SPV_EXT_descriptor_indexing");
    Arguments.push_back(L"-fvk-use-dx-layout"); // Use DirectX-compatible memory layout
    Arguments.push_back(L"-O3"); // Optimization level 3

    Result = Compiler->Compile(
            SourceBlob, std::wstring(loaderRequest.Path.begin(), loaderRequest.Path.end()).c_str(), std::wstring(loaderOptions.EntryPoint.begin(), loaderOptions.EntryPoint.end()).c_str(),
            TargetProfileW.c_str(),Arguments.data(), Arguments.size(), nullptr, 0, nullptr, &OperationResult);
    ensure(!FAILED(Result), "Shader compilation failed.");

    HRESULT CompileStatus;
    Result = OperationResult->GetStatus(&CompileStatus);
    ensure(!FAILED(Result), "Undefined compile status.");

    if (FAILED(CompileStatus))
    {
        CComPtr<IDxcBlobEncoding> ErrorBlob;
        Result = OperationResult->GetErrorBuffer(&ErrorBlob);
        if (SUCCEEDED(Result) && ErrorBlob)
        {
            std::string Exception((char*)ErrorBlob->GetBufferPointer(), ErrorBlob->GetBufferSize());
            RK_LOG_ERROR("Shader Compilation Error: {}", Exception);
        }
    }

    Result = OperationResult->GetResult(&ShaderBlob);
    ensure(!FAILED(Result), "Unable to retrieve compiled shader.");

    const size_t ByteSize = ShaderBlob->GetBufferSize();
    const size_t WordCount = ByteSize / sizeof(uint32_t);

    Asset.Code.resize(WordCount);
    memcpy(Asset.Code.data(), ShaderBlob->GetBufferPointer(), ByteSize);

    ShaderBlob = nullptr;
    Compiler = nullptr;
    Library = nullptr;
    IncludeHandler = nullptr;
    SourceBlob = nullptr;
    OperationResult = nullptr;

    dlclose(DxcLibHandle);

#endif

#ifdef RK_PLATFORM_WINDOWS

#endif
    return Result;
}
