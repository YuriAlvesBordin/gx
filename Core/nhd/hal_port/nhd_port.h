#ifndef NHD_PORT_H
#define NHD_PORT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    void (*write_cmd)    (uint8_t cmd);
    void (*write_data)   (const uint8_t *buf, uint16_t len);
    void (*reset)        (void);
    void (*delay_ms)     (uint32_t ms);
    void (*set_backlight)(bool on);
} NHD_Port;

typedef NHD_Port LCD12864_Port;

#endif /* NHD_PORT_H */
