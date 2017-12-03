#pragma once

#include "types.hh"

struct FontData {
    u32 *textures;
    u32  list_base;
};

class FontManager {
public:
    FontManager();

    FontData *get_font(const char *name, float height);
};
