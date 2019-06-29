#include "APP.h"
#include <stdarg.h>
#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ril.h"
#include "timers.h"
#include "other.h"
#include "opencpu_onenet.h"
#include "OneNet.h"

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

static void getCSQ(void)
{
    static HalTime_t lastTime = 0;
    int rssi, rxqual;

    if(HalTimeHasPast(lastTime, 1000))
    {
        opencpu_csq(&rssi, &rxqual);
        HalPrint("CSQ:%d,%d\n", rssi, rxqual);
        lastTime = HalTime();
    }
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
    OneNetInitialize();
    //OneNetCreate();
}

static void networkHandle(void)
{
    static HalTime_t startConnectTime = 0;

    if(HalNetOnline())
    {
        if(!OneNetConnected() && HalTimeHasPast(startConnectTime, 3500))
        {
            OneNetStartConnect();
            startConnectTime = HalTime();
        }
    }
}

static void valueReport(void)
{
    static HalTime_t lastReportTime = 0;
    if(OneNetConnected() && HalTimeHasPast(lastReportTime, 12000))
    {
        OneNetDataReport("23.5");
        lastReportTime = HalTime();
    }
}

void APPPoll(void)
{
	valueReport();
	networkHandle();
	OneNetPoll();
	getCSQ();
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


