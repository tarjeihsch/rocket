#pragma once

#include "CoreTypes.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <format>
#include <iostream>
#include <thread>
#include <cstring>

enum class ELogLevel : uint8_t
{
    Trace       = 0,
    Debug       = 1,
    Info        = 2,
    Warn        = 3,
    Error       = 4,
    Critical    = 5
};

inline const ANSICHAR* LexToString(ELogLevel LogLevel)
{
    switch (LogLevel)
    {
        case ELogLevel::Trace:    return "TRACE";
        case ELogLevel::Debug:    return "DEBUG";
        case ELogLevel::Info:     return "INFO";
        case ELogLevel::Warn:     return "WARN";
        case ELogLevel::Error:    return "ERROR";
        case ELogLevel::Critical: return "CRIT";
        default:                  return "UNK";
    }
}

inline uint64_t NowNsMonotonic()
{
    return (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

// Cheap per-thread id; replace with platform-specific if you prefer.
inline uint32_t GetThreadIdFast()
{
    static thread_local uint32_t Tid =
        (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
    return Tid;
}

template<uint32_t Capacity, uint32_t MessageCapacity>
class TBufferLogger
{
public:
    static_assert(Capacity > 0, "Capacity must be > 0");
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of two for fast masking");

    struct FLogRecord
    {
        uint64_t TimestampNs = 0;
        ELogLevel Level = ELogLevel::Info;
        uint32_t ThreadId = 0;
        ANSICHAR Message[MessageCapacity] = { 0 };
    };

    struct FDrainResult
    {
        uint64_t Start = 0;
        uint64_t End = 0;

        // True if FromSequence was older than the current ring window.
        bool bLost = false;

        // Exact number of sequences definitely lost due to ring overwrite (relative to FromSequence).
        uint64_t LostCount = 0;

        // Number of sequences in [Start, End) that were skipped because they were not yet published
        // at drain time (producer still writing) OR overwritten before being observed.
        uint64_t SkippedUnreadyOrOverwritten = 0;
    };

private:
    struct alignas(64) FSlot
    {
        // Published sequence for this slot. UINT64_MAX means "nothing published yet".
        std::atomic<uint64_t> PublishedSeq { UINT64_MAX };
        FLogRecord Record;
    };

public:
    TBufferLogger() = default;

    inline void SetMinLevel(const ELogLevel InLevel) noexcept
    {
        MinLevel.store(InLevel, std::memory_order_relaxed);
    }

    inline ELogLevel GetMinLevel() const noexcept
    {
        return MinLevel.load(std::memory_order_relaxed);
    }

    template <typename... TArgs>
    void Log(const ELogLevel InLevel, std::format_string<TArgs...> Format, TArgs&&... Args) noexcept
    {
        if (UNLIKELY(InLevel < MinLevel.load(std::memory_order_relaxed)))
        {
            return;
        }

        // MPSC-safe unique sequence.
        const uint64_t Sequence = WriteSequence.fetch_add(1, std::memory_order_relaxed);
        FSlot& Slot = Slots[Sequence & (Capacity - 1)];

        // Fill record payload.
        FLogRecord& Record = Slot.Record;
        Record.TimestampNs = NowNsMonotonic();
        Record.Level       = InLevel;
        Record.ThreadId    = GetThreadIdFast();

        const auto Result = std::format_to_n(
            Record.Message,
            MessageCapacity - 1,
            Format,
            std::forward<TArgs>(Args)...);

        constexpr char Trunc[] = "...";

        if (Result.size >= MessageCapacity - 1)
        {
            constexpr size_t N = sizeof(Trunc) - 1;
            memcpy(&Record.Message[MessageCapacity - 1 - N], Trunc, N);
        }

        const uint32_t Count =
            (Result.size < (MessageCapacity - 1)) ? (uint32_t)Result.size : (MessageCapacity - 1);

        Record.Message[Count] = '\0';

        // Publish last: release makes payload visible to draining thread that observes the seq.
        Slot.PublishedSeq.store(Sequence, std::memory_order_release);
    }

    // Non-blocking drain: never spins, never waits. Skips anything not immediately readable.
    template <typename FnType>
    FDrainResult Drain(FnType&& Fn, const uint64_t FromSequence = 0) const noexcept
    {
        FDrainResult Out;

        // Snapshot. (Acquire is conservative; per-slot acquire is what matters.)
        const uint64_t End = WriteSequence.load(std::memory_order_acquire);
        Out.End = End;

        if (End == 0)
        {
            return Out;
        }

        const uint64_t Oldest = (End > Capacity) ? (End - Capacity) : 0;

        Out.Start = (FromSequence > Oldest) ? FromSequence : Oldest;
        Out.bLost = (FromSequence < Oldest);
        Out.LostCount = Out.bLost ? (Oldest - FromSequence) : 0;

        for (uint64_t Seq = Out.Start; Seq < End; ++Seq)
        {
            const FSlot& Slot = Slots[Seq & (Capacity - 1)];

            // If it doesn't match now, we do not wait. This is the "avoid stalls at any price" rule.
            const uint64_t Published = Slot.PublishedSeq.load(std::memory_order_acquire);
            if (UNLIKELY(Published != Seq))
            {
                ++Out.SkippedUnreadyOrOverwritten;
                continue;
            }

            Fn(Slot.Record, Seq);
        }

        return Out;
    }

    inline uint64_t GetNextSequence() const noexcept
    {
        return WriteSequence.load(std::memory_order_relaxed);
    }

private:
    alignas(64) std::atomic<uint64_t> WriteSequence { 0 };
    alignas(64) std::atomic<ELogLevel> MinLevel { ELogLevel::Info };
    alignas(64) FSlot Slots[Capacity];
};

namespace Logger
{
    inline TBufferLogger<512, 512> GBufferLog;
    inline uint64_t GFlushCursor = 0;

    inline void DumpGBufferLogger()
    {
        const auto DrainResult = GBufferLog.Drain(
            [](const auto& R, uint64_t Seq)
            {
                ANSICHAR Line[512];

                const auto Result = std::format_to_n(
                    Line,
                    sizeof(Line) - 1,
                    "[{}] Seq={} T={} Th={} {}\n",
                    LexToString(R.Level),
                    Seq,
                    R.TimestampNs,
                    R.ThreadId,
                    R.Message);

                const size_t Count = (Result.size < (sizeof(Line) - 1)) ? Result.size : (sizeof(Line) - 1);
                Line[Count] = '\0';

                std::fputs(Line, stdout);
            },
            GFlushCursor);

        if (DrainResult.bLost)
        {
            std::cout << "Logs were overwritten before flushing. LostCount=" << DrainResult.LostCount << std::endl;
        }

        // Optional diagnostic: indicates contention / partial publishing at drain time.
        // If this is frequently non-zero, producers are still writing while you drain.
        if (DrainResult.SkippedUnreadyOrOverwritten != 0)
        {
            std::cout << "Drain skipped " << DrainResult.SkippedUnreadyOrOverwritten
                      << " records (unpublished at drain time or overwritten)." << std::endl;
        }

        GFlushCursor = DrainResult.End;
    }
}

#define RK_LOG_TRACE(Format, ...)    Logger::GBufferLog.Log(ELogLevel::Trace,    Format __VA_OPT__(,) __VA_ARGS__)
#define RK_LOG_DEBUG(Format, ...)    Logger::GBufferLog.Log(ELogLevel::Debug,    Format __VA_OPT__(,) __VA_ARGS__)
#define RK_LOG_INFO(Format, ...)     Logger::GBufferLog.Log(ELogLevel::Info,     Format __VA_OPT__(,) __VA_ARGS__)
#define RK_LOG_WARN(Format, ...)     Logger::GBufferLog.Log(ELogLevel::Warn,     Format __VA_OPT__(,) __VA_ARGS__)
#define RK_LOG_ERROR(Format, ...)    Logger::GBufferLog.Log(ELogLevel::Error,    Format __VA_OPT__(,) __VA_ARGS__)
#define RK_LOG_CRITICAL(Format, ...) Logger::GBufferLog.Log(ELogLevel::Critical, Format __VA_OPT__(,) __VA_ARGS__)
