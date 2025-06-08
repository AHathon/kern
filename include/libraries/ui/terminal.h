#pragma once

#include "libraries/types.h"

enum {
    FONT_WIDTH     = 8,
    FONT_HEIGHT    = 8,
    FONT_BPG       = 8,  // Bytes per glyph
    FONT_BPL       = 1,  // Bytes per line
    FONT_NUMGLYPHS = 224
};

void Terminal_Init();
void Terminal_Clear();
void Terminal_Write(const char *fmt, ...);