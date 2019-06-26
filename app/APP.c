#include "APP.h"
#include <stdarg.h>
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ril.h"
#include "timers.h"
#include "other.h"
#include "opencpu_onenet.h"

#define APP_SOFTWARE_VERSION "1.0.1"

static void mesgPrint(void)
{
	unsigned char modelVersion[16];
	unsigned char time[32];
	HalPrint("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");	
	HalPrint("----------------------------\n");	
	opencpu_get_base_version(modelVersion);
	HalPrint("Software version %s\n", APP_SOFTWARE_VERSION);
	HalPrint("Model version:%s\n",modelVersion);
	HalPrint("HW version:%s\n",get_band_version() == 0 ? "CM" : "LV");
	HalPrint("Run mode:%d\n", get_run_mode());
	opencpu_rtc_get_time(time);
	HalPrint("Run time: %s\n", time);
	HalPrint("----------------------------\n");	
}
static volatile bool g_sleep = false;
static void rtcTimerCallback()
{
	HalPrint("rtc timer expires\n");
	opencpu_lock_light_sleep();
	
	g_sleep = false;
}

uint32_t g_rtcHandle;
static void fallSleep(void)
{
	HalPrint("fallSleep...\n");
	g_sleep = true;
	opencpu_rtc_timer_create(&g_rtcHandle, 200, false, rtcTimerCallback);
	opencpu_rtc_timer_start(g_rtcHandle);

	opencpu_unlock_light_sleep();
	while(g_sleep);
}

void APPInitialize(void)
{
	HalInitialize();
	mesgPrint();
	if(opencpu_is_boot_from_sleep()==1)
	{

		HalPrint("BOOT CAUSE:WAKE FROM SLEEP\n");
	}
	else
	{
		HalPrint("BOOT CAUSE:POWER_ON OR RESET\n");
	}
	
    opencpu_lock_light_sleep();
}

void APPPoll(void)
{
	static uint32_t lastTime = 0;
	static uint8_t count = 0;
	if(HalTime() - lastTime > 100)
	{
		HalPrint("past 1s...\n");
		lastTime = HalTime();
		count++;
		if(count > 5)
		{
			fallSleep();
			count = 0;
		}
	}
}

void app_task_main(void)
{
	APPInitialize();
	while(1)
	{
		APPPoll();
		vTaskDelay(1);
	}
	vTaskDelete(NULL);
}

/*
 新建opencpu任务，这个函数用户不可更改
*/
void test_opencpu_start()
{
	 xTaskCreate(app_task_main,"opencpu",1024,NULL,TASK_PRIORITY_NORMAL,NULL);
}


