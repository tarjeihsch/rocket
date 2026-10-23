#include "Log.h"

#include <cstdarg>
#include <cstdio>

namespace
{
    constexpr u32 kMessageLength = 2048;

    Log::Sink g_sink = nullptr;
    Log::Sink g_console_sink = nullptr;

    void DefaultSink(Log::Level level, const char* message)
    {
        std::fputs(message, level == Log::Level::Info ? stdout : stderr);
    }

    bool IsSet(Log::Sinks sinks, Log::Sinks sink)
    {
        return (static_cast<u8>(sinks) & static_cast<u8>(sink)) != 0;
    }

    void PrintV(Log::Level level, Log::Sinks sinks, const char* format, va_list arguments)
    {
        char message[kMessageLength];
        std::vsnprintf(message, sizeof(message), format, arguments);

        if (IsSet(sinks, Log::Sinks::Terminal))
        {
            Log::Sink sink = g_sink != nullptr ? g_sink : DefaultSink;
            sink(level, message);
        }

        if (IsSet(sinks, Log::Sinks::Console) && g_console_sink != nullptr)
        {
            g_console_sink(level, message);
        }
    }
}

namespace Log
{
    void SetSink(Sink sink)
    {
        g_sink = sink;
    }

    void SetConsoleSink(Sink sink)
    {
        g_console_sink = sink;
    }

    void Printf(const char* format, ...)
    {
        va_list arguments;
        va_start(arguments, format);
        PrintV(Level::Info, Sinks::Terminal, format, arguments);
        va_end(arguments);
    }

    void Printf(Sinks sinks, const char* format, ...)
    {
        va_list arguments;
        va_start(arguments, format);
        PrintV(Level::Info, sinks, format, arguments);
        va_end(arguments);
    }

    void Warning(const char* format, ...)
    {
        va_list arguments;
        va_start(arguments, format);
        PrintV(Level::Warning, Sinks::Terminal, format, arguments);
        va_end(arguments);
    }

    void Error(const char* format, ...)
    {
        va_list arguments;
        va_start(arguments, format);
        PrintV(Level::Error, Sinks::Terminal, format, arguments);
        va_end(arguments);
    }

    void EnsureFailed(const char* condition, const char* file, int line, const char* format, ...)
    {
        Error("[Ensure] %s failed at %s:%d\n", condition, file, line);

        if (format != nullptr)
        {
            va_list arguments;
            va_start(arguments, format);
            PrintV(Level::Error, Sinks::Terminal, format, arguments);
            va_end(arguments);
            Error("\n");
        }
    }
}
