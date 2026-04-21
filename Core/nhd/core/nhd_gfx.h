#ifndef NHD_GFX_H
#define NHD_GFX_H

#include <stdint.h>
#include <stdbool.h>
#include "nhd_config.h"
#include "../hal_port/nhd_port.h"
#include "nhd_font.h"

typedef enum {
    NHD_COLOR_WHITE  = 0,
    NHD_COLOR_BLACK  = 1,
    NHD_COLOR_INVERT = 2,
} NHD_Color;

typedef enum {
    NHD_ALIGN_LEFT   = 0,
    NHD_ALIGN_CENTER = 1,
    NHD_ALIGN_RIGHT  = 2,
} NHD_LabelAlign;

typedef struct {
    const uint8_t *data;
    uint8_t        width;
    uint8_t        height;
} NHD_Image;

typedef NHD_Color      LCD12864_Color;
typedef NHD_LabelAlign LCD12864_LabelAlign;
typedef NHD_Image      LCD12864_Image;

#define LCD12864_COLOR_WHITE  NHD_COLOR_WHITE
#define LCD12864_COLOR_BLACK  NHD_COLOR_BLACK
#define LCD12864_COLOR_INVERT NHD_COLOR_INVERT
#define LCD12864_ALIGN_LEFT   NHD_ALIGN_LEFT
#define LCD12864_ALIGN_CENTER NHD_ALIGN_CENTER
#define LCD12864_ALIGN_RIGHT  NHD_ALIGN_RIGHT

void    nhd_gfx_init        (const NHD_Port *port);
void    nhd_gfx_clear       (void);
void    nhd_gfx_fill        (NHD_Color color);
void    nhd_gfx_flush       (void);
void    nhd_gfx_set_backlight(bool on);
void    nhd_gfx_draw_pixel  (int16_t x, int16_t y, NHD_Color color);
void    nhd_gfx_draw_polygon(int16_t cx, int16_t cy,
                              const int16_t *pts, uint8_t num_pts,
                              uint16_t angle, uint8_t roundness,
                              bool fill, bool outline, NHD_Color color);
void    nhd_gfx_draw_label  (int16_t cx, int16_t cy,
                              const char *text, const NHD_Font *font,
                              uint16_t angle, uint8_t scale,
                              uint8_t thickness, int8_t spacing,
                              NHD_LabelAlign align, bool invert,
                              NHD_Color color);
void    nhd_gfx_draw_image  (int16_t cx, int16_t cy,
                              const NHD_Image *img,
                              uint16_t angle, uint8_t scale,
                              bool invert, NHD_Color color);
int32_t nhd_sin(uint16_t deg);
int32_t nhd_cos(uint16_t deg);

#define lcd12864_gfx_init          nhd_gfx_init
#define lcd12864_gfx_clear         nhd_gfx_clear
#define lcd12864_gfx_fill          nhd_gfx_fill
#define lcd12864_gfx_flush         nhd_gfx_flush
#define lcd12864_gfx_set_backlight nhd_gfx_set_backlight
#define lcd12864_gfx_draw_pixel    nhd_gfx_draw_pixel
#define lcd12864_gfx_draw_polygon  nhd_gfx_draw_polygon
#define lcd12864_gfx_draw_label    nhd_gfx_draw_label
#define lcd12864_gfx_draw_image    nhd_gfx_draw_image
#define lcd12864_sin               nhd_sin
#define lcd12864_cos               nhd_cos

#endif /* NHD_GFX_H */
