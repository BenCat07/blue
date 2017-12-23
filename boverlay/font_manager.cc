#include "stdafx.hh"

#include "font_manager.hh"

#include <cassert>

#include <FTGL\ftgl.h>

FontManager::FontManager() {
}

FontManager::~FontManager() {
    for (auto &f : fonts) delete f;
}

FontManager::Handle FontManager::get_font(const char *name, u32 height) {
    // Create a pixmap font from a TrueType file.

    auto  new_index = fonts.size();
    auto &font      = fonts.emplace_back(new FTPixmapFont("C:\\Windows\\Fonts\\Calibri.ttf"));

    // If something went wrong, bail out.
    if (font->Error()) return -1;

    // Set the font size and render a small text.
    font->FaceSize(72);

    return new_index;
}

FTFont *FontManager::get_data(const Handle h) {
    assert(h < fonts.size());
    assert(h >= 0);

    return fonts[h];
}
