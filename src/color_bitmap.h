#ifndef __COLOR_BITMAP_H
#define __COLOR_BITMAP_H

typedef struct {
    uint8_t r, g, b;
} RGB_t;

typedef struct {
    int16_t h;
    uint8_t s, v;
} HSV_t;

#define HUE(h)  ({ typeof(h) h1 = h % 360; h1 < 0 ? 360 + h1 : h1; })

void HSV2RGB(HSV_t *hsv, RGB_t *rgb);

#endif //__COLOR_BITMAP_H
