#ifndef HAL_H
#define HAL_H

#include "Ctype.h"
#include "opencpu_api.h"
#include "FreeRTOS.h"
#include "mpu_wrappers.h"

#define HalPrint HalUartPrintf
#define HalTime() xTaskGetTickCount() 
#define HalTimeHasPast(old, past) ((xTaskGetTickCount() - (old)) > past)

void HalInitialize(void);
void HalPoll(void);
#endif

