#pragma once

#include "CoreTypes.h"
#include "Math/Vector.h"

#define KEY_SPACE              32
#define KEY_APOSTROPHE         39  /* ' */
#define KEY_COMMA              44  /* , */
#define KEY_MINUS              45  /* - */
#define KEY_PERIOD             46  /* . */
#define KEY_SLASH              47  /* / */
#define KEY_0                  48
#define KEY_1                  49
#define KEY_2                  50
#define KEY_3                  51
#define KEY_4                  52
#define KEY_5                  53
#define KEY_6                  54
#define KEY_7                  55
#define KEY_8                  56
#define KEY_9                  57
#define KEY_SEMICOLON          59  /* ; */
#define KEY_EQUAL              61  /* = */
#define KEY_A                  65
#define KEY_B                  66
#define KEY_C                  67
#define KEY_D                  68
#define KEY_E                  69
#define KEY_F                  70
#define KEY_G                  71
#define KEY_H                  72
#define KEY_I                  73
#define KEY_J                  74
#define KEY_K                  75
#define KEY_L                  76
#define KEY_M                  77
#define KEY_N                  78
#define KEY_O                  79
#define KEY_P                  80
#define KEY_Q                  81
#define KEY_R                  82
#define KEY_S                  83
#define KEY_T                  84
#define KEY_U                  85
#define KEY_V                  86
#define KEY_W                  87
#define KEY_X                  88
#define KEY_Y                  89
#define KEY_Z                  90
#define KEY_LEFT_BRACKET       91  /* [ */
#define KEY_BACKSLASH          92  /* \ */
#define KEY_RIGHT_BRACKET      93  /* ] */
#define KEY_GRAVE_ACCENT       96  /* ` */
#define KEY_WORLD_1            161 /* non-US #1 */
#define KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define KEY_ESCAPE             256
#define KEY_ENTER              257
#define KEY_TAB                258
#define KEY_BACKSPACE          259
#define KEY_INSERT             260
#define KEY_DELETE             261
#define KEY_RIGHT              262
#define KEY_LEFT               263
#define KEY_DOWN               264
#define KEY_UP                 265
#define KEY_PAGE_UP            266
#define KEY_PAGE_DOWN          267
#define KEY_HOME               268
#define KEY_END                269
#define KEY_CAPS_LOCK          280
#define KEY_SCROLL_LOCK        281
#define KEY_NUM_LOCK           282
#define KEY_PRINT_SCREEN       283
#define KEY_PAUSE              284
#define KEY_F1                 290
#define KEY_F2                 291
#define KEY_F3                 292
#define KEY_F4                 293
#define KEY_F5                 294
#define KEY_F6                 295
#define KEY_F7                 296
#define KEY_F8                 297
#define KEY_F9                 298
#define KEY_F10                299
#define KEY_F11                300
#define KEY_F12                301
#define KEY_LEFT_SHIFT         340
#define KEY_LEFT_CONTROL       341
#define KEY_LEFT_ALT           342
#define KEY_LEFT_SUPER         343
#define KEY_RIGHT_SHIFT        344
#define KEY_RIGHT_CONTROL      345
#define KEY_RIGHT_ALT          346
#define KEY_RIGHT_SUPER        347

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
    MouseWheel,

    Count
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

    uint64_t WindowHandleID = 0;

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
            bool repeat;
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