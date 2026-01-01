#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

struct RHIPipeline;
struct RBuffer;
struct Texture2D;
class RHICommandList;

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

    Shadow,

    UI
};

class RMaterial
{
public:
    explicit RMaterial(EMaterialPass InPass, EMaterialBlendMode InBlendMode, const char* InVertexShaderPath, const char* InPixelShaderPath);
    ~RMaterial();

    void SetBufferParameter(const std::string& Name, RBuffer* Buffer);
    void SetTextureParameter(const std::string& Name, Texture2D* Texture);

    // Called to bind pipeline and write all material parameters to descriptors
    void Bind(RHICommandList& CmdList);

private:
	std::unordered_map<std::string, RBuffer*> Textures;
	std::unordered_map<std::string, Texture2D*> Buffers;

    RHIPipeline* Pipeline;

    const char* VertexShaderPath;
    const char* PixelShaderPath;

    EMaterialPass Pass;
    EMaterialBlendMode BlendMode;
};