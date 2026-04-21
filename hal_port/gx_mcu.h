#ifndef GX_MCU_H
#define GX_MCU_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Platform port vtable.
 * Fill every function pointer before passing to gx_gfx_init().
 */
typedef struct {
    void (*write_cmd) (uint8_t cmd);
    void (*write_data)(const uint8_t *buf, uint16_t len);
    void (*reset)     (void);
    void (*delay_ms)  (uint32_t ms);
    void (*set_backlight)(bool on);
} GX_Mcu;

#endif /* GX_MCU_H */
