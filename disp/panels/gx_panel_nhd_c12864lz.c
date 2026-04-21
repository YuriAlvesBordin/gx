#include "gx_panel_nhd_c12864lz.h"
#include "../controllers/gx_st7565_priv.h"
#include "../../core/gx_gfx.h"
#include "../../core/gx_config.h"

void gx_panel_nhd_c12864lz_init(const GX_Mcu *port)
{
    if (!port) return;

    port->reset();
    port->delay_ms(5u);

    /* Panel-specific init sequence for NHD-C12864LZ */
    port->write_cmd(GX_ST7565_SET_BIAS_9);
    port->write_cmd(GX_ST7565_SET_ADC_NORMAL);
    port->write_cmd(GX_ST7565_SET_COM_REVERSE);
    port->write_cmd(GX_ST7565_SET_ALL_PTS_NORMAL);
    port->write_cmd(GX_ST7565_SET_DISP_NORMAL);
    port->write_cmd((uint8_t)(GX_ST7565_SET_DISP_START_LINE | 0x00u));

    /* Power-on sequence: booster -> regulator -> follower */
    port->write_cmd(GX_ST7565_SET_POWER_CONTROL(0x04u)); port->delay_ms(5u);
    port->write_cmd(GX_ST7565_SET_POWER_CONTROL(0x06u)); port->delay_ms(5u);
    port->write_cmd(GX_ST7565_SET_POWER_CONTROL(0x07u)); port->delay_ms(10u);

    port->write_cmd((uint8_t)(GX_ST7565_SET_RESISTOR_RATIO | 0x05u));
    port->write_cmd(GX_ST7565_SET_VOLUME_FIRST);
    port->write_cmd(GX_CONTRAST);

    /* Hand off to the generic GFX layer */
    gx_gfx_init(port);
}
