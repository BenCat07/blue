#include "stdafx.hh"

#include "overlay_draw.hh"
#include "overlaywindow.hh"

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#endif

#include <cassert>

#ifdef _DEBUG
static auto gl_in_scope = false;
#endif

class GLScope {
public:
    GLScope(int mode) {
#ifdef _DEBUG
        assert(gl_in_scope == false); // if this goes off then you are doing something very wrong
        gl_in_scope = true;
#endif
        glBegin(mode);
    }
    ~GLScope() {
#ifdef _DEBUG
        assert(gl_in_scope == true);
        gl_in_scope = false;
#endif
        glEnd();
    }
};

class DrawManager : public IDrawManager {
    IOverlayWindow *w = nullptr;

public:
    DrawManager(IOverlayWindow *w) : w(w) {}

    void set_draw_color(Color c) override;

    void set_line_width(float new_width) override;
    void draw_line(Vector2 start, Vector2 end) override;
    void draw_polyline(Vector2 *points, int count) override;

    void draw_filled_rect(Vector2 start, Vector2 end) override;
    void draw_outline_rect(Vector2 start, Vector2 end) override;
};

static inline float color_as_float(u8 val) {
    return float(val) / 255.0f;
}

void DrawManager::set_draw_color(Color c) {
    glColor4f(color_as_float(c.r),
              color_as_float(c.g),
              color_as_float(c.b),
              color_as_float(c.a));
}

void DrawManager::set_line_width(float new_width) {
    glLineWidth(new_width);
}

void DrawManager::draw_line(Vector2 start, Vector2 end) {
    auto scope = GLScope(GL_LINES);

    glVertex2f(start.x, start.y);
    glVertex2f(end.x, end.y);
}

void DrawManager::draw_polyline(Vector2 *points, int count) {

    auto scope = GLScope(GL_LINES);

    for (int i = 1; i < count; i++) {
        auto first  = points[i - 1];
        auto second = points[i];
        glVertex2f(first.x, first.y);
        glVertex2f(second.x, second.y);
    }
}

void DrawManager::draw_filled_rect(Vector2 start, Vector2 end) {
    auto scope = GLScope(GL_QUADS);

    glVertex2f(start.x, start.y); // top left
    glVertex2f(end.x, start.y);   // top right
    glVertex2f(end.x, end.y);     // bottom right
    glVertex2f(start.x, end.y);   // bottom left
}

void DrawManager::draw_outline_rect(Vector2 start, Vector2 end) {
    auto scope = GLScope(GL_LINE_LOOP);

    glVertex2f(start.x, start.y); // top left
    glVertex2f(end.x, start.y);   // top right
    glVertex2f(end.x, end.y);     // bottom right
    glVertex2f(start.x, end.y);   // bottom left
}

#ifdef _MSC_VER
__declspec(dllexport) IDrawManager *create_draw_manager(IOverlayWindow *w) {
    return new DrawManager(w);
}
#else
#endif
