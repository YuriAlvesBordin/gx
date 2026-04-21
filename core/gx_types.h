#ifndef GX_TYPES_H
#define GX_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* Pixel color */
typedef enum {
    GX_COLOR_WHITE  = 0,
    GX_COLOR_BLACK  = 1,
    GX_COLOR_INVERT = 2,
} GX_Color;

/* Text alignment */
typedef enum {
    GX_ALIGN_LEFT   = 0,
    GX_ALIGN_CENTER = 1,
    GX_ALIGN_RIGHT  = 2,
} GX_Align;

/* Bitmap image descriptor */
typedef struct {
    const uint8_t *data;
    uint8_t        width;
    uint8_t        height;
} GX_Image;

#endif /* GX_TYPES_H */
