#include "Renderer.h"

#include "Buffer.h"
#include "Texture2D.h"
#include "Common/RHICommandList.h"

static RHICommandList g_CmdList;

void Pass::BindTextureSRV(Texture2D *texture)
{
    RHITextureAccessState texture_access_state = RHITextureAccessState::ShaderRead();
    //cmd_list.enqueue<RHICommandPipelineBarrierTexture>(texture->GetHandle(), texture_access_state);
}

Pass::Pass(RHICommandList &in_cmd_list)
    : cmd_list(in_cmd_list)
{
}

Renderer& Renderer::get()
{
    static Renderer renderer;
    return renderer;
}

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::BeginFrame()
{
}

void Renderer::EndFrame()
{
    GetRHI()->submit(g_CmdList);
}

void Renderer::AddGraphicsPass(const std::function<void(GraphicsPass &)> &callback)
{
    GraphicsPass graphics_pass(g_CmdList);
    callback(graphics_pass);
}

void Renderer::AddComputePass(const std::function<void(ComputePass &)> &callback)
{
    ComputePass compute_pass(g_CmdList);
    callback(compute_pass);
}