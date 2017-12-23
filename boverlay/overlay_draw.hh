#pragma once

#include "types.hh"

#include "font_manager.hh"

class IOverlayWindow;

class IDrawManager {
public:
    virtual class IOverlayWindow *get_overlay() = 0;

    virtual void set_draw_color(Color c) = 0;

    virtual void set_line_width(float new_width)           = 0;
    virtual void draw_line(Vector2 start, Vector2 end)     = 0;
    virtual void draw_polyline(Vector2 *points, int count) = 0;

    virtual void draw_filled_rect(Vector2 start, Vector2 end)  = 0;
    virtual void draw_outline_rect(Vector2 start, Vector2 end) = 0;

    virtual FontManager::Handle create_font(const char *name, u32 size)                              = 0;
    virtual void                draw_text(FontManager::Handle h, Vector2 position, const char *text) = 0;
};

#ifdef _MSC_VER
__declspec(dllexport) IDrawManager *create_draw_manager();
#endif
