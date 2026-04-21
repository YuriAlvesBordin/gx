#ifndef GX_MCU_STM32_H
#define GX_MCU_STM32_H

#include "../gx_mcu.h"
#include "main.h"
#include "spi.h"
#include "gpio.h"

/**
 * Returns a pointer to the ready-to-use STM32 port implementation.
 * Pass the result directly to gx_panel_*_init() or gx_gfx_init().
 */
const GX_Mcu *gx_mcu_stm32_get(void);

/**
 * Call this from HAL_SPI_TxCpltCallback() when SPI2 finishes a DMA transfer.
 */
void gx_mcu_stm32_tx_cplt(void);

#endif /* GX_MCU_STM32_H */
