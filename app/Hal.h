#ifndef HAL_H
#define HAL_H

#include "Ctype.h"
#include "opencpu_api.h"
#include "FreeRTOS.h"
#include "mpu_wrappers.h"

#define HAL_DEVICE_TYPE_TEMPERATURE
//#define HAL_DEVICE_TYPE_GYROSCOPE

#define SECONDS(x) ((x)*100)
#define MINUTES(x) ((x)*SECONDS(60))

#define HalTime_t TickType_t

#define HalPrint HalUartPrintf
#define HalLog(...) do{\
                        HalPrint("%s: ", __FUNCTION__);\
                        HalPrint(__VA_ARGS__); \
                        HalPrint("\n");\
                        }while(0)
#define HalTime() xTaskGetTickCount() 
#define HalTimeHasPast(old, past) ((xTaskGetTickCount() - (old)) > past)

uint16_t HalIntervalGet(void);
void HalIntervalSet(uint16_t value);

void HalReboot(void);
bool HalNetOnline(void);
void HalInitialize(void);
void HalPoll(void);
#endif

