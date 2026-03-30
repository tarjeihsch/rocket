#pragma once

#include <cstdint>
#include <cstddef>

#ifdef _WIN32
    #include <wtypes.h>
#endif

#if defined(__linux__)
    struct wl_display;
    struct wl_surface;

    #include <cstring>
#endif

#include "Math/Vector.h"

namespace RHI 
{
struct GPUShaderResources;
struct GPUShader;
struct GPUTexture;
struct GPUBuffer;

enum class GPUDriver
{
    Unknown,

    OpenGL,

    Vulkan,

    Direct3D12,

    Metal,

        WebGPU
    };

    enum class GPUFormat
    {
        Unknown,

        // 8-bit normalized
        RGBA8_UNORM,
        RGBA8_SRGB,
        BGRA8_UNORM,
        BGRA8_SRGB,

        // 16-bit float
        R16_FLOAT,
        RG16_FLOAT,
        RGBA16_FLOAT,

        // 32-bit float
        R32_FLOAT,
        RG32_FLOAT,
        RGB32_FLOAT,
        RGBA32_FLOAT,

        // 32-bit unsigned integer
        RGBA32_UINT,
        RGBA32_SINT,
        RGB32_UINT,
        RGB32_SINT,
        R32_UINT,

        // Depth / Stencil
        D16_UNORM,
        D32_FLOAT,
        D24_UNORM_S8,
        D32_FLOAT_S8
    };

    enum class GPUAccessType
    {
        Undefined,
        General,
        VertexShaderRead,
        FragmentShaderRead,
        ComputeShaderRead,
        ComputeShaderWrite,
        TransferSrc,
        TransferDst
    };

    enum class GPUTextureLayout
    {
        Undefined,
        General,
        ColorAttachment,
        DepthAttachment,
        ShaderReadOnly,
        TransferSrc,
        TransferDst,
        Present
    };

    enum class GPUPipelineStage
    {
        Top,
        Bottom
    };

    enum class GPUHeapType
    {
        // Invalid state
        Unknown,

        // GPU-local memory (fast for GPU, not accessible by CPU)
        Default,

        // CPU-write, GPU-read memory
        Upload,

        // GPU-write, CPU-read memory
        Readback,
    };

    enum class GPUTextureDimension
    {
        Unknown,
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube
    };

    enum class GPUAdapterVendor
    {
        Unknown     = 0x0,
        AMD         = 0x1002,
        NVIDIA      = 0x10DE,
        APPLE       = 0x106B,
        INTEL       = 0x8086,
    };

    enum class GPUBindGroupEntryType
    {
        Unknown,
        UniformBuffer,
        StorageBuffer,
        SampledTexture,
        StorageTexture,
        Sampler
    };

    enum class GPUPrimitiveTopology
    {
        Unknown,
        TriangleList,
        TriangleStrip,
        LineList,
        LineStrip,
        PointList
    };

    enum class GPUCullMode
    {
        None,
        Front,
        Back
    };

    enum class GPUFrontFace
    {
        Unknown,
        Clockwise,
        CounterClockwise
    };

    enum class GPUPolygonMode
    {
        Unknown,
        Fill,
        Line,
        Point
    };

    enum class GPUPresentMode
    {
        Unknown = 0,
        
        // No VSync, tears possilbe
        Immediate,

        // VSync on, always supported
        FIFO,

        // Adaptive VSync
        FIFOAdaptive,

        // Triple buffering, low latency
        Mailbox
    };

    enum class GPUCompareOp
    {
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always,
    };

    enum class GPUBlendFactor
    {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
    };

    enum class GPUBlendOp
    {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max,
    };

    enum class GPULoadOp
    {
        Unknown,
        Load,
        Clear,
        DontCare,
    };

    enum class GPUStoreOp
    {
        Unknown,
        Store,
        DontCare,
    };

    enum class GPUFilter
    {
        Nearest,
        Linear
    };

    enum class GPUSamplerAddressMode
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder
    };

    enum class GPUBorderColor
    {
        TransparentBlack,
        OpaqueBlack,
        OpaqueWhite
    };
    
    namespace GPUBufferFlag
    {
        enum Bits : uint32_t
        {
            None        = 0U,
            Vertex      = 1U << 0,
            Index       = 1U << 1,
            Storage     = 1U << 2,
            Uniform     = 1U << 3
        };
    };

    typedef uint32_t GPUBufferFlags;

    namespace GPUTextureFlag
    {
        enum Bits : uint32_t
        {
            None = 0U,
            RenderTarget = 1U << 0,
            DepthTarget = 1U << 1,
            SRV = 1U << 2,
            UAV = 1U << 3,
            TransferSrc = 1U << 4,
            TransferDst = 1U << 5
        };
    };

    typedef uint32_t GPUTextureFlags;

    namespace GPUShaderStageFlag
    {
        enum Bits : uint32_t
        {
            None = 0U,
            Vertex = 1U << 0,
            Fragment = 1U << 1,
            Compute = 1U << 2,
            All = Vertex | Fragment | Compute
        };
    };

    typedef uint32_t GPUShaderStageFlags;

    namespace GPUQueueFlag
    {
        enum Bits : uint32_t
        {
            None = 0,
            Graphics = 1U << 0,
            Compute = 1U << 1,
            Transfer = 1U << 2
        };
    }

    typedef uint32_t GPUQueueFlags;

    struct GPUClearValue
    {
        struct ColorClearValue { float r, g, b, a; };
        struct DepthClearValue { float depth; uint32_t stencil; };

        union 
        {
            ColorClearValue color;
            DepthClearValue depth_stencil;
        };

        static GPUClearValue Color(float P_Red = 1.0f, float P_Green = 1.0f, float P_Blue = 1.0f, float P_Alpha = 1.0f)
        {
            GPUClearValue Value;
            Value.color = { P_Red, P_Green, P_Blue, P_Alpha };
            return Value;
        }

        static GPUClearValue Depth(float P_Depth = 0.0f, uint32_t P_Stencil = 0)
        {
            GPUClearValue Value;
            Value.depth_stencil = { P_Depth, P_Stencil };
            return Value;
        }
    };

#if defined(_WIN32)

    struct GPUWindowPlatform
    {
        HWND hwnd = nullptr;
    };

#elif defined(__linux__)
    
    struct GPUWindowPlatform
    {
        wl_display* display = nullptr;
        wl_surface* surface = nullptr;
    };

#endif

    struct GPUTimingQueryResult
    {
        double gpu_time_ms = 0;
    };

    struct GPUPipelineQueryResult
    {
        uint64_t vertices;
        uint64_t primitives;
        uint64_t vertex_invocations;
        uint64_t fragment_invocations;
        uint64_t compute_invocations;
    };

    struct GPUAdapter final
    {
        char device_name[256];
        GPUAdapterVendor vendor = GPUAdapterVendor::Unknown;
        uint32_t device_id;

        float timestamp_period = 0;

        uint32_t max_texture_dimension_2d = 0;
        uint32_t max_texture_dimension_3d = 0;
        uint32_t max_texture_array_layers = 0;
        uint32_t max_color_attachments = 0;
        uint32_t max_bind_groups = 0;
        uint32_t max_bind_group_entries = 0;
        uint32_t max_push_constant_size = 0;
        uint32_t max_compute_workgroup_size[3] = {};
        uint32_t max_compute_workgroup_count[3] = {};
        uint32_t max_sampled_textures = 0;
        uint32_t max_storage_textures = 0;
        uint32_t max_storage_buffers = 0;
        uint32_t max_uniform_buffers = 0;
        uint32_t max_samplers = 0;

        uint8_t bindless_support : 1 = false;
        uint8_t mesh_shader_support : 1 = false;
        uint8_t raytracing_support : 1 = false;
        uint8_t multiview_support : 1 = false;
        uint8_t indirect_draw_support : 1 = false;
        uint8_t timestamp_query_support : 1 = false;
        uint8_t graphics_queue : 1 = false;
        uint8_t compute_queue : 1 = false;
        uint8_t transfer_queue : 1 = false;
    };

    struct GPUColorAttachment
    {        
        GPUTexture* M_Texture;
        GPUClearValue M_Clear_Value;
        GPULoadOp loadOp;
        GPUStoreOp storeOp;
    };

    struct GPUDepthAttachment
    {
        GPUTexture* texture = nullptr;
        GPUClearValue clear_value;
        GPULoadOp load_op = GPULoadOp::Load;
        GPUStoreOp store_op = GPUStoreOp::Unknown;
    };

    struct GPURenderPass
    {
        const GPUColorAttachment* color_attachments[8] = {};
        uint32_t color_attachment_count = 0;

        const GPUDepthAttachment* depth_attachment = nullptr;

        uint32_t render_width = 0;
        uint32_t render_height = 0;
    };

    struct GPUBufferBarrier
    {
        GPUBuffer* buffer = nullptr;
        GPUAccessType src_access = GPUAccessType::Undefined;
        GPUAccessType dst_access = GPUAccessType::Undefined;
    };

    struct GPUTextureBarrier
    {
        GPUTexture* texture = nullptr;
        GPUTextureLayout old_layout = GPUTextureLayout::Undefined;
        GPUTextureLayout new_layout = GPUTextureLayout::Undefined;
    };

    struct GPUBufferDesc
    {
        uint64_t size = 0;
        GPUHeapType heap_type = GPUHeapType::Unknown;
        GPUBufferFlags flags = GPUBufferFlag::None;

        static GPUBufferDesc DefaultBuffer(uint64_t size, GPUBufferFlags flags = GPUBufferFlag::None)
        {
            GPUBufferDesc desc;
            desc.size = size;
            desc.heap_type = GPUHeapType::Default;
            desc.flags = flags;
            return desc;
        }

        static GPUBufferDesc UploadBuffer(uint64_t size, GPUBufferFlags flags = GPUBufferFlag::None)
        {
            GPUBufferDesc desc;
            desc.size = size;
            desc.heap_type = GPUHeapType::Upload;
            desc.flags = flags;
            return desc;
        }

        static GPUBufferDesc ReadbackBuffer(uint64_t size, GPUBufferFlags flags = GPUBufferFlag::None)
        {
            GPUBufferDesc desc;
            desc.size = size;
            desc.heap_type = GPUHeapType::Readback;
            desc.flags = flags;
            return desc;
        }

        static GPUBufferDesc VertexBuffer(uint64_t size)
        {
            return DefaultBuffer(size, GPUBufferFlag::Vertex);
        }

        static GPUBufferDesc IndexBuffer(uint64_t size)
        {
            return DefaultBuffer(size, GPUBufferFlag::Index);
        }

        static GPUBufferDesc StorageBuffer(uint64_t size)
        {
            return UploadBuffer(size, GPUBufferFlag::Storage);
        }

        static GPUBufferDesc UniformBuffer(uint64_t size)
        {
            return UploadBuffer(size, GPUBufferFlag::Uniform);
        }
    };

    struct GPUTextureDesc
    {
        GPUFormat format = GPUFormat::Unknown;
        GPUTextureFlags flags = GPUTextureFlag::None;
        GPUTextureDimension dimension = GPUTextureDimension::Unknown;

        const GPUClearValue* clear_value = nullptr;

        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depth = 1;
        uint32_t mip_level_count = 1;
        uint32_t layer_count = 1;
        uint32_t sample_count = 1;

        static GPUTextureDesc Texture1D(GPUFormat format, uint32_t width, GPUTextureFlags flags = GPUTextureFlag::SRV, uint32_t mip_level_count = 1)
        {
            GPUTextureDesc desc;
            desc.format = format;
            desc.flags = flags;
            desc.dimension = GPUTextureDimension::Texture1D;
            desc.width = width;
            desc.mip_level_count = mip_level_count;
            return desc;
        }

        static GPUTextureDesc Texture2D(GPUFormat format, uint32_t width, uint32_t height, GPUTextureFlags flags = GPUTextureFlag::SRV, uint32_t mip_level_count = 1, uint32_t sample_count = 1)
        {
            GPUTextureDesc desc;
            desc.format = format;
            desc.flags = flags;
            desc.dimension = GPUTextureDimension::Texture2D;
            desc.width = width;
            desc.height = height;
            desc.mip_level_count = mip_level_count;
            desc.sample_count = sample_count;
            return desc;
        }

        static GPUTextureDesc Texture3D(GPUFormat format, uint32_t width, uint32_t height, uint32_t depth, GPUTextureFlags flags = GPUTextureFlag::SRV, uint32_t mip_level_count = 1)
        {
            GPUTextureDesc desc;
            desc.format = format;
            desc.flags = flags;
            desc.dimension = GPUTextureDimension::Texture3D;
            desc.width = width;
            desc.height = height;
            desc.depth = depth;
            desc.mip_level_count = mip_level_count;
            return desc;
        }

        static GPUTextureDesc TextureCube(GPUFormat format, uint32_t size, GPUTextureFlags flags = GPUTextureFlag::SRV, uint32_t mip_level_count = 1)
        {
            GPUTextureDesc desc;
            desc.format = format;
            desc.flags = flags;
            desc.dimension = GPUTextureDimension::TextureCube;
            desc.width = size;
            desc.height = size;
            desc.layer_count = 6;
            desc.mip_level_count = mip_level_count;
            return desc;
        }

        static GPUTextureDesc Texture2DArray(GPUFormat format, uint32_t width, uint32_t height, uint32_t layer_count, GPUTextureFlags flags = GPUTextureFlag::SRV, uint32_t mip_level_count = 1)
        {
            GPUTextureDesc desc;
            desc.format = format;
            desc.flags = flags;
            desc.dimension = GPUTextureDimension::Texture2D;
            desc.width = width;
            desc.height = height;
            desc.layer_count = layer_count;
            desc.mip_level_count = mip_level_count;
            return desc;
        }

        static GPUTextureDesc RenderTarget(GPUFormat format, uint32_t width, uint32_t height, uint32_t sample_count = 1)
        {
            return Texture2D(format, width, height, GPUTextureFlag::RenderTarget | GPUTextureFlag::SRV | GPUTextureFlag::TransferSrc | GPUTextureFlag::TransferDst, 1, sample_count);
        }

        static GPUTextureDesc DepthTarget(GPUFormat format, uint32_t width, uint32_t height, uint32_t sample_count = 1)
        {
            return Texture2D(format, width, height, GPUTextureFlag::DepthTarget | GPUTextureFlag::SRV | GPUTextureFlag::TransferSrc | GPUTextureFlag::TransferDst, 1, sample_count);
        }
    };

    struct GPUSamplerDesc
    {
        GPUFilter minFilter = GPUFilter::Nearest;
        GPUFilter magFilter = GPUFilter::Nearest;
        GPUFilter mipFilter = GPUFilter::Nearest;

        GPUSamplerAddressMode addressU = GPUSamplerAddressMode::Repeat;
        GPUSamplerAddressMode addressV = GPUSamplerAddressMode::Repeat;
        GPUSamplerAddressMode addressW = GPUSamplerAddressMode::Repeat;

        float mipLodBias = 0.0f;
        float minLod = 0.0f;
        float maxLod = 0.0f;
        float maxAnisotropy = 1.0f;

        GPUCompareOp compareOp = GPUCompareOp::Never;
        GPUBorderColor borderColor = GPUBorderColor::TransparentBlack; 

        uint8_t anisotropyEnable : 1 = false;
        uint8_t compareEnable : 1 = false;

        static GPUSamplerDesc PointClamp()
        {
            GPUSamplerDesc samplerDesc;
            memset(&samplerDesc, 0, sizeof(GPUSamplerDesc));
            samplerDesc.minFilter = GPUFilter::Nearest;
            samplerDesc.magFilter = GPUFilter::Nearest;
            samplerDesc.mipFilter = GPUFilter::Nearest;
            samplerDesc.addressU  = GPUSamplerAddressMode::ClampToEdge;
            samplerDesc.addressV  = GPUSamplerAddressMode::ClampToEdge;
            samplerDesc.addressW  = GPUSamplerAddressMode::ClampToEdge;
            return samplerDesc;
        }

        static GPUSamplerDesc LinearClamp()
        {
            GPUSamplerDesc samplerDesc;
            memset(&samplerDesc, 0, sizeof(GPUSamplerDesc));
            samplerDesc.minFilter = GPUFilter::Linear;
            samplerDesc.magFilter = GPUFilter::Linear;
            samplerDesc.mipFilter = GPUFilter::Linear;
            samplerDesc.addressU  = GPUSamplerAddressMode::ClampToEdge;
            samplerDesc.addressV  = GPUSamplerAddressMode::ClampToEdge;
            samplerDesc.addressW  = GPUSamplerAddressMode::ClampToEdge;
            return samplerDesc;
        }

        static GPUSamplerDesc LinearRepeat()
        {
            GPUSamplerDesc samplerDesc;
            memset(&samplerDesc, 0, sizeof(GPUSamplerDesc));
            samplerDesc.minFilter = GPUFilter::Linear;
            samplerDesc.magFilter = GPUFilter::Linear;
            samplerDesc.mipFilter = GPUFilter::Linear;
            return samplerDesc;
        }
    };

    struct GPUGraphicsPipelineDesc
    {
        const GPUShader* vertex_shader = nullptr;
        const GPUShader* fragment_shader = nullptr;
        
        GPUPrimitiveTopology topology = GPUPrimitiveTopology::Unknown;
        GPUCullMode cull_mode = GPUCullMode::Back;
        GPUFrontFace front_face = GPUFrontFace::CounterClockwise;
        GPUPolygonMode polygon_mode = GPUPolygonMode::Fill;

        const GPUFormat* color_formats = nullptr;
        uint32_t color_format_count = 0;
        
        GPUFormat depth_format = GPUFormat::Unknown;
        GPUCompareOp depth_compare_op = GPUCompareOp::Less;

        GPUBlendFactor src_blend_factor_color = GPUBlendFactor::One;
        GPUBlendFactor dst_blend_factor_color = GPUBlendFactor::Zero;
        GPUBlendOp blend_op_color = GPUBlendOp::Add;

        GPUBlendFactor src_blend_factor_alpha = GPUBlendFactor::One;
        GPUBlendFactor dst_blend_factor_alpha = GPUBlendFactor::Zero;
        GPUBlendOp blend_op_alpha = GPUBlendOp::Add;

        uint8_t depth_test_enable : 1 = false;
        uint8_t depth_write_enable : 1 = false;
        uint8_t blend_enable : 1 = false;
    };

    struct GPUComputePipelineDesc
    {
        const GPUShader* computeShader = nullptr;
    };

    struct GPUSwapchainDesc
    {
        GPUWindowPlatform window_platform;

        uint32_t backbuffer_width = 0;
        uint32_t backbuffer_height = 0;
        GPUFormat backbuffer_format = GPUFormat::Unknown;

        uint32_t backbuffer_count = 0;
        GPUPresentMode present_mode = GPUPresentMode::Unknown;
    };

    struct GPUShaderDesc
    {
        const void* bytecode = nullptr;
        uint64_t bytecode_size = 0;
        GPUShaderStageFlag::Bits stage_flag = GPUShaderStageFlag::None;
    };

    struct GPUCommandListDesc
    {
        GPUQueueFlags queue_flags;
    };

    constexpr uint32_t GetFormatBytesPerPixel(GPUFormat format)
    {
        switch (format)
        {
            case GPUFormat::D16_UNORM:          return 2;
            case GPUFormat::R16_FLOAT:          return 2;
            case GPUFormat::RGBA8_UNORM:        return 4;
            case GPUFormat::RGBA8_SRGB:         return 4;
            case GPUFormat::BGRA8_UNORM:        return 4;
            case GPUFormat::BGRA8_SRGB:         return 4;
            case GPUFormat::RG16_FLOAT:         return 4;
            case GPUFormat::R32_FLOAT:          return 4;
            case GPUFormat::R32_UINT:           return 4;
            case GPUFormat::D32_FLOAT:          return 4;
            case GPUFormat::D24_UNORM_S8:       return 4;
            case GPUFormat::D32_FLOAT_S8:       return 5;
            case GPUFormat::RG32_FLOAT:         return 8;
            case GPUFormat::RGBA16_FLOAT:       return 8;
            case GPUFormat::RGB32_FLOAT:        return 12;
            case GPUFormat::RGB32_UINT:         return 12;
            case GPUFormat::RGB32_SINT:         return 12;
            case GPUFormat::RGBA32_FLOAT:       return 16;
            case GPUFormat::RGBA32_UINT:        return 16;
            case GPUFormat::RGBA32_SINT:        return 16;
            default:                            return 0;
        }
    }
}
