#pragma once

#include "types.hh"

#include <vector>

#define FTGL_LIBRARY_STATIC

class FTPixmapFont;
class FTFont;

class FontManager {

    std::vector<FTPixmapFont *> fonts;

public:
    using Handle = u32;

    FontManager();
    ~FontManager();

    Handle get_font(const char *name, u32 height);

    FTFont *get_data(const Handle h);
};
