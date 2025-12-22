#pragma once

#include "CoreTypes.h"
#include "Math/Vector2.h"

namespace Platform
{
    enum class EventType : uint8_t
    {
        None = 0,

        Quit,

        WindowCloseRequested,
        WindowResized,
        WindowMoved,
        WindowFocusGained,
        WindowFocusLost,

        KeyDown,
        KeyUp,
        TextInput,

        MouseMove,
        MouseButtonDown,
        MouseButtonUp,
        MouseWheel
    };

    enum class Button : uint8_t
    {
        Unknown = 0,
        Left,
        Middle,
        Right,
        X1,
        X2,
    };

    enum class Key : uint8_t
    {
        Unknown = 0,

        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        Num0, Num1, Num2, Num3, Num4, 
        Num5, Num6, Num7, Num8, Num9,

        Escape,
        Enter,
        Tab,
        Backspace,
        Space,

        Left, Right, Up, Down,

        Insert, Delete, Home,
        End, PageUp, PageDown,

        LeftShift, RightShift,
        LeftCtrl, RightCtrl,
        LeftAlt, RightAlt,
        LeftMeta, RightMeta,

        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
    };

    struct Modifier
    {
        uint8_t isShiftDown = false;
        uint8_t isCtrlDown = false;
        uint8_t isAltDown = false;
        uint8_t isMetaDown = false;
        uint8_t isCapsLockOn = false;
        uint8_t isNumLockOn = false;
    };

    struct TextInput
    {
        ANSICHAR buffer[64];
    };

    struct Event
    {
        EventType type = EventType::None;

        uint64_t windowHandle = 0;

        union
        {
            struct 
            {
                Vector2i position;
                Vector2i delta;
            } axis;

            struct
            {
                Button value;
                Modifier modifier;
                Vector2i position;
            } button;

            struct
            {
                Vector2f value;
            } wheel;

            struct
            {
                Key value;
                Modifier modifier;
                bool repeat;
            } key;

            struct
            {
                Vector2i value;
            } windowResize;

            struct
            {
                Vector2i value;
            } windowMove;

            struct
            {
                TextInput value;
            } text;
        };
    };

    class EventSink
    {
    public:
        virtual ~EventSink() = default;

        virtual void Enqueue(const Event& event) = 0;
        virtual bool Dequeue(Event& event) = 0;
    };
}
