#ifndef GX_ST7565_PRIV_H
#define GX_ST7565_PRIV_H

/* ST7565 command set */
#define GX_ST7565_DISPLAY_OFF          0xAEu
#define GX_ST7565_DISPLAY_ON           0xAFu
#define GX_ST7565_SET_DISP_START_LINE  0x40u
#define GX_ST7565_SET_PAGE             0xB0u
#define GX_ST7565_SET_COL_HI           0x10u
#define GX_ST7565_SET_COL_LO           0x00u
#define GX_ST7565_SET_ADC_NORMAL       0xA0u
#define GX_ST7565_SET_ADC_REVERSE      0xA1u
#define GX_ST7565_SET_DISP_NORMAL      0xA6u
#define GX_ST7565_SET_DISP_REVERSE     0xA7u
#define GX_ST7565_SET_ALL_PTS_NORMAL   0xA4u
#define GX_ST7565_SET_ALL_PTS_ON       0xA5u
#define GX_ST7565_SET_BIAS_9           0xA2u
#define GX_ST7565_SET_BIAS_7           0xA3u
#define GX_ST7565_INTERNAL_RESET       0xE2u
#define GX_ST7565_SET_COM_NORMAL       0xC0u
#define GX_ST7565_SET_COM_REVERSE      0xC8u
#define GX_ST7565_SET_POWER_CONTROL(x) (0x28u | ((x) & 0x07u))
#define GX_ST7565_SET_RESISTOR_RATIO   0x20u
#define GX_ST7565_SET_VOLUME_FIRST     0x81u
#define GX_ST7565_SET_VOLUME(x)        ((x) & 0x3Fu)
#define GX_ST7565_NOP                  0xE3u

#endif /* GX_ST7565_PRIV_H */
