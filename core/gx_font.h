#ifndef GX_FONT_H
#define GX_FONT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    const uint8_t *data;
    uint8_t        width;
    uint8_t        height;
    uint8_t        spacing;
    char           first_char;
    uint8_t        char_count;
} GX_Font;

/* Built-in 5×7 font */
extern const GX_Font gx_font_5x7;

/**
 * Retrieve the bitmap for a Unicode codepoint from the 5×7 font.
 * Returns true and sets *glyph, *width, *pages on success.
 */
bool gx_font_5x7_get_glyph(uint16_t codepoint,
                            const uint8_t **glyph,
                            uint8_t        *width,
                            uint8_t        *pages);

#endif /* GX_FONT_H */
