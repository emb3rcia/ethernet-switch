#ifndef KSZSPI_H
#define KSZSPI_H

#include "main.h"

void KSZ_SPI_Init(void);
void KSZ_WriteReg8(uint16_t address, uint8_t data);
uint8_t KSZ_ReadReg8(uint16_t address);

#endif /* KSZSPI_H */