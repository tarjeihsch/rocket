#pragma once

#include "CoreTypes.h"

#include <memory>

class RHIPipeline;

class Pipeline 
{
public:
	RHIPipeline* GetRHIPipeline() const
	{
		return rhi_pipeline;
	}

protected:
	RHIPipeline* rhi_pipeline;
};

class GraphicsPipeline : public Pipeline 
{
public:
	explicit GraphicsPipeline(const ANSICHAR* vertex_shader_ir_path, const ANSICHAR* pixel_shader_ir_path);
};

class ComputePipeline : public Pipeline
{
public:
	explicit ComputePipeline(const ANSICHAR* compute_shader_ir_path);
};