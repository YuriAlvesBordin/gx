#ifndef NHDC12864LZ_H
#define NHDC12864LZ_H

#include "../nhd_port.h"

/**
 * @brief  Initialises the NHD-C12864LZ-FSW-FBW display.
 *
 * Sends the full ST7565 power-on sequence (bias, ADC, COM direction,
 * power control ramp, contrast, clear) and turns the display on.
 *
 * @param  port  Pointer to a filled NHD_Port struct with the platform
 *               callbacks (write_cmd, write_data, reset, delay_ms).
 *               Must not be NULL.
 */
void nhdc12864LZ_init(const NHD_Port *port);

#define lcd12864LZ_init nhdc12864LZ_init

#endif /* NHDC12864LZ_H */
