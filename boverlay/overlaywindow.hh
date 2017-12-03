#pragma once

#include "types.hh"

using OverlayWindowOnFrameCallback = void (*)(void *userdata);

class IOverlayWindow {
public:
    virtual bool init()     = 0;
    virtual void shutdown() = 0;

#ifdef _MSC_VER
    // target should be a HWND of the target window you want to overlay
    virtual void set_target(void *target) = 0;
#else
    virtual void set_target() = 0;
#endif
    virtual void frame() = 0;

    virtual void set_on_frame_callback(OverlayWindowOnFrameCallback f, void *userdata = nullptr) = 0;

    virtual Vector2 size() = 0;
};

#ifdef _MSC_VER
__declspec(dllexport) IOverlayWindow *create_overlay_window();
#else
IOverlay *create_overlay_window();
#endif
