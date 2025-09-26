#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

struct FRHIBuffer;
struct FRHITextureView;
struct RBuffer;
struct FTexture2D;

enum class EMaterialBlendMode
{
    // Fully opaque, depth writes on, blending off
    Opaque,

    // Alpha test (cutout), still writes to depth
    Masked,

    // Alpha blended, depth write off, depth test on
    Transparent,

    // Glow effects, particles
    Additive
};

enum class EMaterialPass
{
    Lit,

    DepthOnly,

    Shadow
};

class FRHIPipeline;

enum class LMaterialResourceType : uint8_t
{
    Texture,
    Buffer
};

class RMaterial
{
public:
    RMaterial(const char* VertexShader, const char* PixelShaderPath);
    ~RMaterial();

    void SetBufferParameter(const std::string& Name, RBuffer* Buffer);
    void SetTextureParameter(const std::string& Name, FTexture2D* Texture);

    // Called to write all material parameters to descriptors
    void Bind();

private:
    FRHIPipeline* Pipeline;

    std::unordered_map<std::string, RBuffer*> Textures;
    std::unordered_map<std::string, FTexture2D*> Buffers;
};