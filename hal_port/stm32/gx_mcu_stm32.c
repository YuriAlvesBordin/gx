#include "gx_mcu_stm32.h"
#include <stdbool.h>

extern SPI_HandleTypeDef hspi2;

static volatile bool s_dma_busy = false;

#define GX_SPI_TIMEOUT_MS 10u

static bool wait_dma_ready(void)
{
    if (!s_dma_busy) return true;
    uint32_t start = HAL_GetTick();
    while (s_dma_busy) {
        if ((HAL_GetTick() - start) >= GX_SPI_TIMEOUT_MS) {
            HAL_SPI_Abort(&hspi2);
            HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_SET);
            s_dma_busy = false;
            return false;
        }
    }
    return true;
}

static void write_cmd(uint8_t cmd)
{
    if (!wait_dma_ready()) return;
    HAL_GPIO_WritePin(DISPLAY_A0_GPIO_Port, DISPLAY_A0_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_RESET);
    if (HAL_SPI_Transmit(&hspi2, &cmd, 1, GX_SPI_TIMEOUT_MS) == HAL_OK) {
        uint32_t start = HAL_GetTick();
        while (__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_BSY))
            if ((HAL_GetTick() - start) >= GX_SPI_TIMEOUT_MS) { HAL_SPI_Abort(&hspi2); break; }
    }
    HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_SET);
}

static void write_data(const uint8_t *buf, uint16_t len)
{
    if (!buf || !len) return;
    if (!wait_dma_ready()) return;
    s_dma_busy = true;
    HAL_GPIO_WritePin(DISPLAY_A0_GPIO_Port, DISPLAY_A0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_RESET);
    if (HAL_SPI_Transmit_DMA(&hspi2, (uint8_t *)buf, len) != HAL_OK) {
        HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_SET);
        s_dma_busy = false;
    }
}

static void do_reset(void)
{
    HAL_GPIO_WritePin(DISPLAY_RST_GPIO_Port, DISPLAY_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(DISPLAY_RST_GPIO_Port, DISPLAY_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

static void delay_ms(uint32_t ms) { HAL_Delay(ms); }

static void set_backlight(bool on)
{
    HAL_GPIO_WritePin(DISPLAY_BKLT_GPIO_Port, DISPLAY_BKLT_Pin,
                      on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static const GX_Mcu s_port = {
    .write_cmd     = write_cmd,
    .write_data    = write_data,
    .reset         = do_reset,
    .delay_ms      = delay_ms,
    .set_backlight = set_backlight,
};

const GX_Mcu *gx_mcu_stm32_get(void)
{
    return &s_port;
}

void gx_mcu_stm32_tx_cplt(void)
{
    /* Called from DMA IRQ context — avoid HAL_GetTick(); use a simple counter. */
    uint32_t timeout = 100000u;
    while (__HAL_SPI_GET_FLAG(&hspi2, SPI_FLAG_BSY) && --timeout);
    HAL_GPIO_WritePin(DISPLAY_CS_GPIO_Port, DISPLAY_CS_Pin, GPIO_PIN_SET);
    s_dma_busy = false;
}
