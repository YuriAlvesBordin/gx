#ifndef NHD_PORT_STM32_H
#define NHD_PORT_STM32_H

#include "../../nhd_port.h"
#include "main.h"
#include "spi.h"
#include "gpio.h"

/* Returns a ready-to-use STM32 port; pass to nhd_gfx_init(). */
const NHD_Port *nhd_port_stm32_get(void);

/* Call from HAL_SPI_TxCpltCallback() when SPI2 DMA transfer finishes. */
void nhd_port_stm32_tx_cplt(void);

#define lcd12864_port_stm32_get     nhd_port_stm32_get
#define lcd12864_port_stm32_tx_cplt nhd_port_stm32_tx_cplt

#endif /* NHD_PORT_STM32_H */
