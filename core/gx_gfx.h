#ifndef GX_GFX_H
#define GX_GFX_H

#include <stdint.h>
#include <stdbool.h>
#include "gx_config.h"
#include "gx_types.h"
#include "gx_font.h"
#include "../hal_port/gx_mcu.h"

/**
 * Initialise the renderer with a platform port.
 * Must be called before any draw function.
 */
void gx_gfx_init(const GX_Mcu *port);

/* Framebuffer control */
void gx_gfx_clear(void);
void gx_gfx_fill(GX_Color color);
void gx_gfx_flush(void);
void gx_gfx_set_backlight(bool on);

/* Primitives */
void gx_gfx_draw_pixel(int16_t x, int16_t y, GX_Color color);

/**
 * Draw a convex or concave polygon.
 * pts  : interleaved x,y pairs (numpts * 2 values)
 * angle: rotation in degrees [0..359]
 * roundness: line thickness / corner radius
 */
void gx_gfx_draw_polygon(int16_t cx, int16_t cy,
                          const int16_t *pts, uint8_t numpts,
                          uint16_t angle, uint8_t roundness,
                          bool fill, bool outline,
                          GX_Color color);

/**
 * Draw a UTF-8 string.
 * scale    : Q4 fixed-point (GX_SCALE_1X = 1×)
 * thickness: stroke width in pixels
 * spacing  : extra inter-glyph pixels (can be negative)
 */
void gx_gfx_draw_label(int16_t cx, int16_t cy,
                        const char *text, const GX_Font *font,
                        uint16_t angle, uint8_t scale,
                        uint8_t thickness, int8_t spacing,
                        GX_Align align, bool invert,
                        GX_Color color);

/**
 * Draw a bitmap image.
 * scale: Q4 fixed-point (GX_SCALE_1X = 1×)
 */
void gx_gfx_draw_image(int16_t cx, int16_t cy,
                        const GX_Image *img,
                        uint16_t angle, uint8_t scale,
                        bool invert, GX_Color color);

/* Trig helpers (Q8 fixed-point, 256 = 1.0) */
int32_t gx_sin(uint16_t deg);
int32_t gx_cos(uint16_t deg);

#endif /* GX_GFX_H */
