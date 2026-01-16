#pragma once

#include <string>
#include <vector>

#include "RenderGraphTypes.h"

struct RenderGraphPass
{
    std::string name;
    QueueType queue_type = QueueType::Graphics;
    PassExecuteFunc execute_func;

    std::vector<TextureBarrierInfo> texture_barriers;
    std::vector<BufferBarrierInfo> buffer_barriers;

    std::vector<TextureHandle> texture_reads;
    std::vector<TextureHandle> texture_writes;

    std::vector<BufferHandle> buffer_reads;
    std::vector<BufferHandle> buffer_writes;

    std::vector<RHISemaphore*> wait_semaphores;
    std::vector<RHISemaphore*> signal_semaphores;
};
