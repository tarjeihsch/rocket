#pragma once

#include <CoreTypes.h>

// Low-level logging that any runtime module can call without depending on the
// console. The console installs itself as the Console sink at startup; until
// then, and in tools that do not link the console, a message sent to it is
// dropped. The Terminal sink falls back to stdout/stderr.
//
// The message is formatted exactly like printf, and a message may contain
// newlines: each line becomes a separate console line.

namespace Log
{
    enum class Level : u8
    {
        Info,
        Warning,
        Error
    };

    // Where a message goes. Combine them with | to send it to both.
    enum class Sinks : u8
    {
        Terminal = 1 << 0,
        Console = 1 << 1
    };

    inline Sinks operator|(Sinks left, Sinks right)
    {
        return static_cast<Sinks>(static_cast<u8>(left) | static_cast<u8>(right));
    }

    using Sink = void (*)(Level level, const char* message);

    // Passing nullptr restores the default stdout/stderr sink.
    void SetSink(Sink sink);

    // Passing nullptr unregisters the console's sink.
    void SetConsoleSink(Sink sink);

    // Without sinks, the message goes to the terminal.
    void Printf(const char* format, ...);
    void Printf(Sinks sinks, const char* format, ...);
    void Warning(const char* format, ...);
    void Error(const char* format, ...);

    // Reports a failed ensure. The optional format and arguments add a custom
    // message on the following line.
    void EnsureFailed(const char* condition, const char* file, int line, const char* format = nullptr, ...);
}
