#ifndef NHD_FONT_H
#define NHD_FONT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    const uint8_t *data;
    uint8_t        width;
    uint8_t        height;
    uint8_t        spacing;
    char           first_char;
    char           char_count;
} NHD_Font;

typedef NHD_Font LCD12864_Font;

extern const NHD_Font nhd_font_5x7;

/* Returns the bitmap for one Unicode codepoint when supported by the built-in 5x7 font. */
bool nhd_font_5x7_get_glyph(uint16_t codepoint, const uint8_t **glyph,
                              uint8_t *width, uint8_t *pages);

#define lcd12864_font_5x7           nhd_font_5x7
#define lcd12864_font_5x7_get_glyph nhd_font_5x7_get_glyph

#endif /* NHD_FONT_H */
