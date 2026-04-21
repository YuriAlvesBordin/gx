#ifndef GX_PANEL_NHD_C12864LZ_H
#define GX_PANEL_NHD_C12864LZ_H

#include "../../hal_port/gx_mcu.h"

/**
 * Initialise the NHD-C12864LZ panel (ST7565 controller).
 * Calls gx_gfx_init() internally — do NOT call it separately.
 */
void gx_panel_nhd_c12864lz_init(const GX_Mcu *port);

#endif /* GX_PANEL_NHD_C12864LZ_H */
