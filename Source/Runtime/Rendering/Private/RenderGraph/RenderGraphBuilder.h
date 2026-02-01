#pragma once

#include "RenderGraphTypes.h"

class RenderGraphPass;

class RenderGraphBuilder
{
public:
    explicit RenderGraphBuilder(RenderGraphPass& in_pass);

    void ReadTexture(TextureHandle handle);
    void WriteTexture(TextureHandle handle);

    void WriteColorAttachment(TextureHandle handle);
    void WriteDepthAttachment(TextureHandle handle);

    void ReadBuffer(BufferHandle handle);
    void WriteBuffer(BufferHandle handle);

    void CopySrcTexture(TextureHandle handle);
    void CopyDstTexture(TextureHandle handle);

    void Present(TextureHandle BackBuffer);

private:
    RenderGraphPass& pass;
};