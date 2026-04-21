#ifndef NHD_ST7565_PRIV_H
#define NHD_ST7565_PRIV_H

#define ST7565_DISPLAY_OFF           0xAE
#define ST7565_DISPLAY_ON            0xAF
#define ST7565_SET_DISP_START_LINE   0x40
#define ST7565_SET_PAGE              0xB0
#define ST7565_SET_COL_HI            0x10
#define ST7565_SET_COL_LO            0x00
#define ST7565_SET_ADC_NORMAL        0xA0
#define ST7565_SET_ADC_REVERSE       0xA1
#define ST7565_SET_DISP_NORMAL       0xA6
#define ST7565_SET_DISP_REVERSE      0xA7
#define ST7565_SET_ALLPTS_NORMAL     0xA4
#define ST7565_SET_ALLPTS_ON         0xA5
#define ST7565_SET_BIAS_9            0xA2
#define ST7565_SET_BIAS_7            0xA3
#define ST7565_INTERNAL_RESET        0xE2
#define ST7565_SET_COM_NORMAL        0xC0
#define ST7565_SET_COM_REVERSE       0xC8
#define ST7565_SET_POWER_CONTROL(x)  (0x28 | ((x) & 0x07))
#define ST7565_SET_RESISTOR_RATIO(x) (0x20 | ((x) & 0x07))
#define ST7565_SET_VOLUME_FIRST      0x81
#define ST7565_SET_VOLUME(x)         ((x) & 0x3F)
#define ST7565_NOP                   0xE3

#endif /* NHD_ST7565_PRIV_H */
