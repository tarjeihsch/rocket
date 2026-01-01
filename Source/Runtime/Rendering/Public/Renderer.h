#pragma once

#include <functional>

#include "Texture2D.h"
#include "Math/Vector2.h"

enum class TextureState : uint8_t;
class RHICommandList;
struct Buffer;
struct Texture2D;
struct RHIBufferAccessState;
struct RHIBuffer;
struct RHITextureAccessState;
struct RHITexture;
struct RHIPipeline;

class Pass
{
public:
    void BindPipeline(RHIPipeline* pipeline);

    void BindTextureSRV(Texture2D* texture);
    void BindTextureUAV(Texture2D* texture);
    void BindTextureRenderTarget(Texture2D* texture);
    void BindTextureDepthStencil(Texture2D* texture);
    void BindTextureDepthReadOnly(Texture2D* texture);
    void BindTextureCopySrc(Texture2D* texture);
    void BindTextureCopyDst(Texture2D* texture);
    void BindTexturePresent(Texture2D* texture);

    void BindBufferSRV(Buffer* buffer);
    void BindBufferUAV(Buffer* buffer);
    void BindBufferCopySrc(Buffer* buffer);
    void BindBufferCopyDst(Buffer* buffer);
    void BindVertexBuffer(Buffer* buffer);
    void BindIndexBuffer(Buffer* buffer);
    void BindIndirectBuffer(Buffer* buffer);

protected:
    explicit Pass(RHICommandList& in_cmd_list);

    RHIPipeline* pending_pipeline;

    Texture2D* pending_texture_srv;
    Texture2D* pending_texture_uav;
    Texture2D* pending_texture_render_target;
    Texture2D* pending_texture_depth_stencil;
    Texture2D* pending_texture_depth_readonly;
    Texture2D* pending_texture_copy_src;
    Texture2D* pending_texture_copy_dst;
    Texture2D* pending_texture_present;

protected:
    RHICommandList& cmd_list;
};

class GraphicsPass : public Pass
{
public:
    explicit GraphicsPass(RHICommandList& in_cmd_list)
        : Pass(in_cmd_list)
    {
    }

    void SetScissor(Vector2i position, Vector2i extent);
    void Draw();
};

class ComputePass : public Pass
{
public:
    explicit ComputePass(RHICommandList& in_cmd_list)
        : Pass(in_cmd_list)
    {
    }

    void Dispatch();
};

class Renderer
{
public:
    static Renderer& get();

    Renderer();
    ~Renderer();

    void BeginFrame();
    void EndFrame();

    void AddGraphicsPass(const std::function<void(GraphicsPass&)> &callback);
    void AddComputePass(const std::function<void(ComputePass&)> &callback);
};

//void game()
//{
//    Texture2D texture = Texture2D(ETextureType::Linear, 1280, 720);
//
//    Renderer::get().BeginFrame();
//
//    Renderer::get().AddGraphicsPass([&](GraphicsPass& pass)
//    {
//        pass.UseTexture(&texture, TextureState::UAV);
//        pass.Draw();
//        pass.UseTexture(&texture, TextureState::Present);
//    });
//
//    Renderer::get().EndFrame();
//}
