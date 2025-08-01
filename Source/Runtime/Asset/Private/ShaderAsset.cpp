#include "ShaderAsset.h"

#include "Assert.h"

#ifdef RK_PLATFORM_LINUX
    #include <dlfcn.h>
    #include <dxc/dxcapi.h>
#endif


ShaderAsset TAssetLoader<ShaderAsset>::Load(const FAssetLoaderRequest &pRequest, const ShaderAssetOptions& pOptions)
{
    void* DxcLibHandle = dlopen(ROCKET_BINARIES "/ThirdParty/Vulkan/lib/libdxcompiler.so", RTLD_LAZY);
    RK_ASSERT(DxcLibHandle, "Failed to dynamically load DXC library (Ensure DXC is installed and environment is set).");

    using DxcCreateInstanceProc = HRESULT(__stdcall*)(const IID&, const IID&, void**);
    auto DxcCreateInstance = reinterpret_cast<DxcCreateInstanceProc>(dlsym(DxcLibHandle, "DxcCreateInstance"));
    RK_ASSERT(DxcCreateInstance, "Failed to locate DxcCreateInstance function address.");

    CComPtr<IDxcBlob> ShaderBlob;
    CComPtr<IDxcCompiler> Compiler;
    CComPtr<IDxcLibrary> Library;
    CComPtr<IDxcIncludeHandler> IncludeHandler;
    CComPtr<IDxcBlobEncoding> SourceBlob;
    CComPtr<IDxcOperationResult> OperationResult;

    // Create the DXC library instance
    HRESULT Result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler));
    RK_ASSERT(!FAILED(Result), "Failed to create DXC Library instance.");

    // Create the DXC compiler instance
    Result = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&Library));
    RK_ASSERT(!FAILED(Result), "Failed to create DXC Compiler instance.");

    // Load and encode the shader source file
    Result = Library->CreateBlobFromFile(std::wstring(pRequest.Path.begin(), pRequest.Path.end()).c_str(), nullptr, &SourceBlob);
    RK_ASSERT(!FAILED(Result), "Failed to load shader source file.");

    std::wstring TargetProfileW(pOptions.TargetProfile.begin(), pOptions.TargetProfile.end());
    std::vector<LPCWSTR> Arguments;
    Arguments.push_back(L"-T");
    Arguments.push_back(TargetProfileW.c_str());
    Arguments.push_back(L"-spirv");
    Arguments.push_back(L"-fspv-target-env=vulkan1.3");
    Arguments.push_back(L"-fspv-extension=SPV_KHR_physical_storage_buffer");
    Arguments.push_back(L"-fvk-use-dx-layout"); // Use DirectX-compatible memory layout
    Arguments.push_back(L"-O3"); // Optimization level 3

    Result = Compiler->Compile(
            SourceBlob, std::wstring(pRequest.Path.begin(), pRequest.Path.end()).c_str(), std::wstring(pOptions.EntryPoint.begin(), pOptions.EntryPoint.end()).c_str(),
            TargetProfileW.c_str(),Arguments.data(), Arguments.size(), nullptr, 0, nullptr, &OperationResult);
    RK_ASSERT(!FAILED(Result), "Shader compilation failed.");

    HRESULT CompileStatus;
    Result = OperationResult->GetStatus(&CompileStatus);
    RK_ASSERT(!FAILED(Result), "Undefined compile status.");

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
    RK_ASSERT(!FAILED(Result), "Unable to retrieve compiled shader.");

    const size_t ByteSize = ShaderBlob->GetBufferSize();
    const size_t WordCount = ByteSize / sizeof(uint32_t);

    ShaderAsset Asset;
    Asset.Code.resize(WordCount);
    memcpy(Asset.Code.data(), ShaderBlob->GetBufferPointer(), ByteSize);

    ShaderBlob = nullptr;
    Compiler = nullptr;
    Library = nullptr;
    IncludeHandler = nullptr;
    SourceBlob = nullptr;
    OperationResult = nullptr;

    dlclose(DxcLibHandle);

    return Asset;
}