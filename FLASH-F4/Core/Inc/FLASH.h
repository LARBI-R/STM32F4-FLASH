#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f4xx_hal.h"



uint8_t FLASH_Unlock(void);
void FLASH_Lock(void);
void FLASH_LockOpt(void);
uint8_t FLASH_UnlockOpt(void);
HAL_StatusTypeDef FLASH_Write(uint32_t SectorAdd, void * data, uint8_t size, uint16_t length);
HAL_StatusTypeDef FLASH_Read(uint32_t SectorID, void * RxBuf, uint8_t size, uint16_t length);
#endif /* __FLASH_H */ 
