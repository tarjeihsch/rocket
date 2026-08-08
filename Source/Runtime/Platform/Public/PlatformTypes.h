#pragma once

enum class ENativeHandleKind
{
    // App connection context
    WindowInstance,

    // Native window
    Window
};

using RNativeWindowSystemHandle = void*;
using RNativeWindowHandle = void*;

enum class ENativePlatformBackend
{
    Win32,
    X11,
    Wayland,
    Cocoa
};