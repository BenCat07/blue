#include <cstdio>

#include "../boverlay/overlay_draw.hh"
#include "../boverlay/overlaywindow.hh"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma comment(lib, "boverlay.lib")

void on_frame(void *draw_manager) {
    auto dm = static_cast<IDrawManager *>(draw_manager);

    dm->set_draw_color({255, 0, 0, 255});
    dm->draw_line({0, 0}, {500, 500});

    Vector2 points[] = {
        {0, 0},
        {100, 50},
        {200, 100},
        {250, 200},
        {300, 300},
        {300, 400},
    };

    dm->set_draw_color({0, 255, 255, 255});
    dm->draw_polyline(points, sizeof(points) / sizeof(Vector2));

    dm->set_draw_color({0, 255, 0, 127});
    dm->draw_filled_rect({0, 500}, {300, 600});

    dm->set_draw_color({255, 255, 0, 255});
    dm->draw_outline_rect({0, 400}, {200, 700});

    //static auto font_handle = dm->create_font("C:\\Windows\\Fonts\\Calibri.ttf", 12);
    //dm->draw_text(font_handle, {0, 0}, "wow nice meme");
}

int main() {
    auto draw_manager = create_draw_manager();
    auto overlay      = draw_manager->get_overlay();

    auto target = FindWindow("Notepad", nullptr);
    overlay->set_target(target);

    overlay->set_on_frame_callback(&on_frame, draw_manager);

    for (int i = 0;; ++i) {
        overlay->frame();
        Sleep(10);
    }

    return 0;
}
