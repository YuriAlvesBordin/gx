#ifndef GX_FONT_H
#define GX_FONT_H

#include <stdint.h>
#include <stdbool.h>

/* Font descriptor */
typedef struct {
    const uint8_t *data;
    uint8_t        width;
    uint8_t        height;
    uint8_t        spacing;
    char           first_char;
    char           char_count;
} GX_Font;

/* Built-in 5x7 font instance */
extern const GX_Font gx_font_5x7;

/**
 * Retrieve the bitmap for one Unicode codepoint from the 5x7 font.
 * Returns true if the glyph is supported.
 */
bool gx_font_5x7_get_glyph(uint16_t codepoint,
                            const uint8_t **glyph,
                            uint8_t *width,
                            uint8_t *pages);

#endif /* GX_FONT_H */
