#ifndef GX_CONFIG_H
#define GX_CONFIG_H

/* Display geometry */
#define GX_WIDTH         128
#define GX_HEIGHT         64
#define GX_PAGES         (GX_HEIGHT / 8)
#define GX_BUF_SIZE      (GX_WIDTH * GX_PAGES)
#define GX_COL_OFFSET      0

/* Default contrast (0x00–0x3F) */
#define GX_CONTRAST      0x0B

/* Polygon / label limits */
#define GX_MAX_POLY_PTS   16
#define GX_LABEL_MAX_CP  192

/* Scale: Q4 fixed-point  (16 = 1×, 8 = 0.5×, 32 = 2×) */
#define GX_SCALE_1X       16
#define GX_SCALE_MIN       2

/* Static widget pools (set GX_USE_POOL 0 to use malloc instead) */
#define GX_USE_POOL        1
#define GX_POOL_SHAPES    16
#define GX_POOL_LABELS     8
#define GX_POOL_IMAGES     8

#endif /* GX_CONFIG_H */
