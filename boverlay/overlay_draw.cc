#include "stdafx.hh"

#include "overlay_draw.hh"
#include "overlaywindow.hh"

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#endif

#include "FTGL/ftgl.h"

#include "glm/glm.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <cassert>

// helper for compiling shaders
static i32 compile_shader(const char *source, u32 type) {
    i32 status;
    i32 shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE) {
        char    infolog[512];
        GLsizei length;
        glGetShaderInfoLog(shader, 512, &length, infolog);
        printf("boverlay: Shader compile error: %s\n", infolog);
        assert(status == GL_TRUE);
    }

    return shader;
}

const char *shader_vertex =
    "#version 130\n"
    "\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "in vec2 vertex;\n"
    "in vec2 tex_coord;\n"
    "in vec4 color;\n"
    "in int drawmode;\n"
    "flat out int frag_DrawMode;\n"
    "out vec4 frag_Color;\n"
    "out vec2 frag_TexCoord;\n"
    "void main()\n"
    "{\n"
    "    frag_TexCoord = tex_coord;\n"
    "    frag_Color    = color;\n"
    "    gl_Position   = projection*(view*(model*vec4(vertex,0.0,1.0)));\n"
    "    frag_DrawMode = drawmode;\n"
    "}";

const char *shader_fragment =
    "#version 130\n"
    "\n"
    "uniform sampler2D texture;\n"
    "in vec4 frag_Color;\n"
    "in vec2 frag_TexCoord;\n"
    "flat in int frag_DrawMode;\n"
    "void main()\n"
    "{\n"
    "   if (frag_DrawMode == 1)\n"
    "       gl_FragColor = frag_Color;\n"
    "   else\n"
    "   {\n"
    "       vec4 tex = texture2D(texture, frag_TexCoord);\n"
    "       if (frag_DrawMode == 2)\n"
    "           gl_FragColor = frag_Color * tex;\n"
    "       else if (frag_DrawMode == 3)\n"
    "       {\n"
    "           if (tex.r > 0.4) tex.r = 1.0;\n"
    "           gl_FragColor = vec4(frag_Color.rgb, frag_Color.a * tex.r);\n"
    "       }\n"
    "       else\n"
    "           gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "    }\n"
    "}";

class DrawManager : public IDrawManager {
    IOverlayWindow *w;

    FontManager font_manager;

    u32 shader;

public:
    DrawManager();

    IOverlayWindow *get_overlay() override { return w; }

    void set_draw_color(Color c) override;

    void set_line_width(float new_width) override;
    void draw_line(Vector2 start, Vector2 end) override;
    void draw_polyline(Vector2 *points, int count) override;

    void draw_filled_rect(Vector2 start, Vector2 end) override;
    void draw_outline_rect(Vector2 start, Vector2 end) override;

    FontManager::Handle create_font(const char *name, u32 size) override;
    void                draw_text(FontManager::Handle h, Vector2 position, const char *text) override;
};

DrawManager::DrawManager() : w(create_overlay_window(this)), font_manager(), shader(0) {
    w->init();

    w->make_current();

    shader = glCreateProgram();

    i32 fragment_shader = compile_shader(shader_fragment, GL_FRAGMENT_SHADER);
    glAttachShader(shader, fragment_shader);

    i32 vertex_shader = compile_shader(shader_vertex, GL_VERTEX_SHADER);
    glAttachShader(shader, vertex_shader);

    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);

    glLinkProgram(shader);

    i32 status;
    glGetProgramiv(shader, GL_LINK_STATUS, &status);
    assert(status == true);

    glUseProgram(status);

    glm::mat4 model, view, projection;

    model = view = projection = glm::mat4x4(1.0);

    auto size = w->size();

    projection = glm::ortho(0.0f, float(size.x), float(size.y), 0.0f);

    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, 0, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, 0, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, 0, glm::value_ptr(projection));

    glUseProgram(0);
}

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

FontManager::Handle DrawManager::create_font(const char *name, u32 size) {
    return font_manager.get_font(name, size);
}

void DrawManager::draw_text(FontManager::Handle h, Vector2 position, const char *text) {
    auto font = font_manager.get_data(h);
    assert(font);

    font->Render(text, -1, FTPoint(position.x, position.y));
}

#ifdef _MSC_VER
__declspec(dllexport) IDrawManager *create_draw_manager() {
    return new DrawManager();
}
#else
#endif
