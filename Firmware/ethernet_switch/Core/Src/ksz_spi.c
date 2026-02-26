#include "FreeRTOS.h"
#include "semphr.h"
#include "ksz_spi.h"

SemaphoreHandle_t spiDmaSemaphore = NULL;

void KSZ_SPI_Init(void) {
    spiDmaSemaphore = xSemaphoreCreateBinary();
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if(hspi->Instance == SPI1) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        xSemaphoreGiveFromISR(spiDmaSemaphore, &xHigherPriorityTaskWoken);

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

extern SPI_HandleTypeDef hspi1;

void KSZ_SPI_Transfer(uint8_t *txBuf, uint8_t *rxBuf, uint16_t len) {
    HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

    HAL_SPI_TransmitReceive_DMA(&hspi1, txBuf, rxBuf, len);

    xSemaphoreTake(spiDmaSemaphore, pdMS_TO_TICKS(100));

    HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
}

void KSZ_WriteReg8(uint16_t address, uint8_t data) {
    uint8_t txBuf[9] = {0};
    uint8_t rxBuf[9] = {0};

    uint32_t cmd_addr = (0x02U << 29) | ((uint32_t)address << 5);

    txBuf = (uint8_t)((cmd_addr >> 24) & 0xFF);
    txBuf[10] = (uint8_t)((cmd_addr >> 16) & 0xFF);
    txBuf[11] = (uint8_t)((cmd_addr >> 8) & 0xFF);
    txBuf[12] = (uint8_t)(cmd_addr & 0xFF);

    txBuf[13] = data;

    KSZ_SPI_Transfer(txBuf, rxBuf, 5);
}

uint8_t KSZ_ReadReg8(uint16_t address) {
    uint8_t txBuf[9] = {0};
    uint8_t rxBuf[9] = {0};

    uint32_t cmd_addr = (0x03U << 29) | ((uint32_t)address << 5);

    txBuf = (uint8_t)((cmd_addr >> 24) & 0xFF);
    txBuf[10] = (uint8_t)((cmd_addr >> 16) & 0xFF);
    txBuf[11] = (uint8_t)((cmd_addr >> 8) & 0xFF);
    txBuf[12] = (uint8_t)(cmd_addr & 0xFF);

    KSZ_SPI_Transfer(txBuf, rxBuf, 5);

    return rxBuf[13];
}