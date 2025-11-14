#pragma once

#include "Viewport.h"

#include <memory>

class RTexture2D;

class REditorViewportClient : public RViewportClient
{
public:
    REditorViewportClient();
    ~REditorViewportClient();

	virtual void Draw(RFrameContext& FrameCtx) override;

private:
    void Init();
    void Shutdown();

private:
	std::unique_ptr<RTexture2D> FontAtlas;
};