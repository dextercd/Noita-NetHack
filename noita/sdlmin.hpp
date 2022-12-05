#define SDL_RELEASED 0
#define SDL_PRESSED 1

typedef enum
{
    SDL_FIRSTEVENT     = 0,
    SDL_QUIT           = 0x100,
    SDL_APP_TERMINATING,
    SDL_APP_LOWMEMORY,
    SDL_APP_WILLENTERBACKGROUND,
    SDL_APP_DIDENTERBACKGROUND,
    SDL_APP_WILLENTERFOREGROUND,
    SDL_APP_DIDENTERFOREGROUND,
    SDL_WINDOWEVENT    = 0x200,
    SDL_SYSWMEVENT,
    SDL_KEYDOWN        = 0x300,
    SDL_KEYUP,
    SDL_TEXTEDITING,
    SDL_TEXTINPUT,
    SDL_KEYMAPCHANGED,
    SDL_MOUSEMOTION    = 0x400,
    SDL_MOUSEBUTTONDOWN,
    SDL_MOUSEBUTTONUP,
    SDL_MOUSEWHEEL,
    SDL_JOYAXISMOTION  = 0x600,
    SDL_JOYBALLMOTION,
    SDL_JOYHATMOTION,
    SDL_JOYBUTTONDOWN,
    SDL_JOYBUTTONUP,
    SDL_JOYDEVICEADDED,
    SDL_JOYDEVICEREMOVED,
    SDL_CONTROLLERAXISMOTION  = 0x650,
    SDL_CONTROLLERBUTTONDOWN,
    SDL_CONTROLLERBUTTONUP,
    SDL_CONTROLLERDEVICEADDED,
    SDL_CONTROLLERDEVICEREMOVED,
    SDL_CONTROLLERDEVICEREMAPPED,
    SDL_FINGERDOWN      = 0x700,
    SDL_FINGERUP,
    SDL_FINGERMOTION,
    SDL_DOLLARGESTURE   = 0x800,
    SDL_DOLLARRECORD,
    SDL_MULTIGESTURE,
    SDL_CLIPBOARDUPDATE = 0x900,
    SDL_DROPFILE        = 0x1000,
    SDL_DROPTEXT,
    SDL_DROPBEGIN,
    SDL_DROPCOMPLETE,
    SDL_AUDIODEVICEADDED = 0x1100,
    SDL_AUDIODEVICEREMOVED,
    SDL_RENDER_TARGETS_RESET = 0x2000,
    SDL_RENDER_DEVICE_RESET,
    SDL_USEREVENT    = 0x8000,
    SDL_LASTEVENT    = 0xFFFF
} SDL_EventType;

typedef struct SDL_CommonEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
} SDL_CommonEvent;

typedef struct SDL_WindowEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::uint32_t windowID;
    std::uint8_t event;
    std::uint8_t padding1;
    std::uint8_t padding2;
    std::uint8_t padding3;
    std::int32_t data1;
    std::int32_t data2;
} SDL_WindowEvent;

enum SDL_Scancode;

typedef struct SDL_Keysym
{
    SDL_Scancode scancode;
    std::int32_t sym;
    std::uint16_t mod;
    std::uint32_t unused;
} SDL_Keysym;

typedef struct SDL_KeyboardEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::uint32_t windowID;
    std::uint8_t state;
    std::uint8_t repeat;
    std::uint8_t padding2;
    std::uint8_t padding3;
    SDL_Keysym keysym;
} SDL_KeyboardEvent;

#define SDL_TEXTEDITINGEVENT_TEXT_SIZE (32)
typedef struct SDL_TextEditingEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::uint32_t windowID;
    char text[SDL_TEXTEDITINGEVENT_TEXT_SIZE];
    std::int32_t start;
    std::int32_t length;
} SDL_TextEditingEvent;


#define SDL_TEXTINPUTEVENT_TEXT_SIZE (32)
typedef struct SDL_TextInputEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::uint32_t windowID;
    char text[SDL_TEXTINPUTEVENT_TEXT_SIZE];
} SDL_TextInputEvent;

typedef struct SDL_MouseMotionEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::uint32_t windowID;
    std::uint32_t which;
    std::uint32_t state;
    std::int32_t x;
    std::int32_t y;
    std::int32_t xrel;
    std::int32_t yrel;
} SDL_MouseMotionEvent;

typedef struct SDL_MouseButtonEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::uint32_t windowID;
    std::uint32_t which;
    std::uint8_t button;
    std::uint8_t state;
    std::uint8_t clicks;
    std::uint8_t padding1;
    std::int32_t x;
    std::int32_t y;
} SDL_MouseButtonEvent;

typedef struct SDL_MouseWheelEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::uint32_t windowID;
    std::uint32_t which;
    std::int32_t x;
    std::int32_t y;
    std::uint32_t direction;
} SDL_MouseWheelEvent;

typedef struct SDL_JoyAxisEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::int32_t which;
    std::uint8_t axis;
    std::uint8_t padding1;
    std::uint8_t padding2;
    std::uint8_t padding3;
    std::int16_t value;
    std::uint16_t padding4;
} SDL_JoyAxisEvent;

typedef struct SDL_JoyBallEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::int32_t which;
    std::uint8_t ball;
    std::uint8_t padding1;
    std::uint8_t padding2;
    std::uint8_t padding3;
    std::int16_t xrel;
    std::int16_t yrel;
} SDL_JoyBallEvent;

typedef struct SDL_JoyHatEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::int32_t which;
    std::uint8_t hat;
    std::uint8_t value;
    std::uint8_t padding1;
    std::uint8_t padding2;
} SDL_JoyHatEvent;

typedef struct SDL_JoyButtonEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::int32_t which;
    std::uint8_t button;
    std::uint8_t state;
    std::uint8_t padding1;
    std::uint8_t padding2;
} SDL_JoyButtonEvent;

typedef struct SDL_JoyDeviceEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::int32_t which;
} SDL_JoyDeviceEvent;


typedef struct SDL_ControllerAxisEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::int32_t which;
    std::uint8_t axis;
    std::uint8_t padding1;
    std::uint8_t padding2;
    std::uint8_t padding3;
    std::int16_t value;
    std::uint16_t padding4;
} SDL_ControllerAxisEvent;


typedef struct SDL_ControllerButtonEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::int32_t which;
    std::uint8_t button;
    std::uint8_t state;
    std::uint8_t padding1;
    std::uint8_t padding2;
} SDL_ControllerButtonEvent;


typedef struct SDL_ControllerDeviceEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::int32_t which;
} SDL_ControllerDeviceEvent;

typedef struct SDL_AudioDeviceEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::uint32_t which;
    std::uint8_t iscapture;
    std::uint8_t padding1;
    std::uint8_t padding2;
    std::uint8_t padding3;
} SDL_AudioDeviceEvent;


typedef struct SDL_TouchFingerEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::int64_t touchId;
    std::int64_t fingerId;
    float x;
    float y;
    float dx;
    float dy;
    float pressure;
} SDL_TouchFingerEvent;


typedef struct SDL_MultiGestureEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::uint64_t touchId;
    float dTheta;
    float dDist;
    float x;
    float y;
    std::uint16_t numFingers;
    std::uint16_t padding;
} SDL_MultiGestureEvent;


typedef struct SDL_DollarGestureEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::int64_t touchId;
    std::int64_t gestureId;
    std::uint32_t numFingers;
    float error;
    float x;
    float y;
} SDL_DollarGestureEvent;


typedef struct SDL_DropEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    char *file;
    std::uint32_t windowID;
} SDL_DropEvent;


typedef struct SDL_QuitEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
} SDL_QuitEvent;

typedef struct SDL_OSEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
} SDL_OSEvent;

typedef struct SDL_UserEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    std::uint32_t windowID;
    std::int32_t code;
    void *data1;
    void *data2;
} SDL_UserEvent;


struct SDL_SysWMmsg;
typedef struct SDL_SysWMmsg SDL_SysWMmsg;

typedef struct SDL_SysWMEvent
{
    std::uint32_t type;
    std::uint32_t timestamp;
    SDL_SysWMmsg *msg;
} SDL_SysWMEvent;

typedef union SDL_Event
{
    std::uint32_t type;
    SDL_CommonEvent common;
    SDL_WindowEvent window;
    SDL_KeyboardEvent key;
    SDL_TextEditingEvent edit;
    SDL_TextInputEvent text;
    SDL_MouseMotionEvent motion;
    SDL_MouseButtonEvent button;
    SDL_MouseWheelEvent wheel;
    SDL_JoyAxisEvent jaxis;
    SDL_JoyBallEvent jball;
    SDL_JoyHatEvent jhat;
    SDL_JoyButtonEvent jbutton;
    SDL_JoyDeviceEvent jdevice;
    SDL_ControllerAxisEvent caxis;
    SDL_ControllerButtonEvent cbutton;
    SDL_ControllerDeviceEvent cdevice;
    SDL_AudioDeviceEvent adevice;
    SDL_QuitEvent quit;
    SDL_UserEvent user;
    SDL_SysWMEvent syswm;
    SDL_TouchFingerEvent tfinger;
    SDL_MultiGestureEvent mgesture;
    SDL_DollarGestureEvent dgesture;
    SDL_DropEvent drop;

    std::uint8_t padding[56];
} SDL_Event;


typedef enum
{
    SDL_ADDEVENT,
    SDL_PEEKEVENT,
    SDL_GETEVENT
} SDL_eventaction;

typedef enum
{
    KMOD_NONE = 0x0000,
    KMOD_LSHIFT = 0x0001,
    KMOD_RSHIFT = 0x0002,
    KMOD_LCTRL = 0x0040,
    KMOD_RCTRL = 0x0080,
    KMOD_LALT = 0x0100,
    KMOD_RALT = 0x0200,
    KMOD_LGUI = 0x0400,
    KMOD_RGUI = 0x0800,
    KMOD_NUM = 0x1000,
    KMOD_CAPS = 0x2000,
    KMOD_MODE = 0x4000,
    KMOD_RESERVED = 0x8000
} SDL_Keymod;

#define SDL_QUERY   -1
#define SDL_IGNORE   0
#define SDL_DISABLE  0
#define SDL_ENABLE   1
