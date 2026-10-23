#include "Log.h"

#include <cstdarg>
#include <cstdio>

namespace
{
    constexpr u32 kMessageLength = 2048;

    Log::Sink g_sink = nullptr;

    void DefaultSink(Log::Level level, const char* message)
    {
        std::fputs(message, level == Log::Level::Info ? stdout : stderr);
    }

    void PrintV(Log::Level level, const char* format, va_list arguments)
    {
        char message[kMessageLength];
        std::vsnprintf(message, sizeof(message), format, arguments);

        Log::Sink sink = g_sink != nullptr ? g_sink : DefaultSink;
        sink(level, message);
    }
}

namespace Log
{
    void SetSink(Sink sink)
    {
        g_sink = sink;
    }

    void Printf(const char* format, ...)
    {
        va_list arguments;
        va_start(arguments, format);
        PrintV(Level::Info, format, arguments);
        va_end(arguments);
    }

    void Warning(const char* format, ...)
    {
        va_list arguments;
        va_start(arguments, format);
        PrintV(Level::Warning, format, arguments);
        va_end(arguments);
    }

    void Error(const char* format, ...)
    {
        va_list arguments;
        va_start(arguments, format);
        PrintV(Level::Error, format, arguments);
        va_end(arguments);
    }

    void EnsureFailed(const char* condition, const char* file, int line, const char* format, ...)
    {
        Error("[Ensure] %s failed at %s:%d\n", condition, file, line);

        if (format != nullptr)
        {
            va_list arguments;
            va_start(arguments, format);
            PrintV(Level::Error, format, arguments);
            va_end(arguments);
            Error("\n");
        }
    }
}
