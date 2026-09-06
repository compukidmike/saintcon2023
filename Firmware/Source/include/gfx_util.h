#ifndef _GFX_UTIL_H_
#define _GFX_UTIL_H_

#include <Arduino_GFX_Library.h>

/** Transparent-color blit — matches GFX API used by original SC23 sources. */
inline void gfxDraw16bitRGBBitmapWithTranColor(
    Arduino_GFX *g, int16_t x, int16_t y,
    const uint16_t *bitmap, uint16_t transparent_color,
    int16_t w, int16_t h) {
    for (int16_t row = 0; row < h; ++row) {
        for (int16_t col = 0; col < w; ++col) {
            uint16_t c = bitmap[row * w + col];
            if (c != transparent_color)
                g->drawPixel(x + col, y + row, c);
        }
    }
}

#endif
