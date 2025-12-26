#pragma once

#include <cstdint>

enum class ERHIQueueSyncStage : uint8_t
{
    TopOfPipe,
    ColorAttachmentOutput,
    AllCommands
};

struct FRHISemaphore
{
    virtual ~FRHISemaphore() = default;
};