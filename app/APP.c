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

static bool g_startSleep = false;
static HalTime_t g_startSleepTime;

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
	HalPrint("Report Interval: %d min\n", HalIntervalGet());
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
	opencpu_rtc_timer_create(&g_rtcHandle, 3000, false, rtcTimerCallback);
	opencpu_rtc_timer_start(g_rtcHandle);

	opencpu_unlock_light_sleep();
	opencpu_entersleep_mode();
	while(1);
}

static void getCSQ(void)
{
    static HalTime_t lastTime = 0;
    int rssi, rxqual;

    if(HalTimeHasPast(lastTime, SECONDS(10)))
    {
        opencpu_csq(&rssi, &rxqual);
        HalPrint("CSQ:%d,%d\n", rssi, rxqual);
        lastTime = HalTime();
    }
}

static void getICCID(void)
{
	unsigned char buf[30];
	int i = 0;

	buf[0] = '\0';
	while(opencpu_iccid(buf)!= 0)
	{
		i++;
		vTaskDelay(10);
		if(i>20)
		{
			HalPrint("iccid timeout\n");
			HalReboot();
			return;
		}
	}
	HalPrint("ICCID:%s\n",buf);
	
	memset(buf, 0, sizeof(buf));
	opencpu_get_imei(buf);
	HalPrint("IMEI:%s\n", buf);
	memset(buf, 0, sizeof(buf));
	opencpu_get_imsi(buf);
	HalPrint("IMSI:%s\n", buf);
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
	getICCID();
    OneNetInitialize();
    //OneNetCreate();
}

static void setSleepMode(void)
{
	static bool set = false;

	if(!set)
	{
		HalLog("");
		ril_power_saving_mode_setting_req_t psm_req1;
		psm_req1.mode=1;
		psm_req1.req_prdc_rau=NULL;
		psm_req1.req_gprs_rdy_tmr=NULL;
		psm_req1.req_prdc_tau="00101011";
		psm_req1.req_act_time="00100100";
		 
		opencpu_set_psmparam(&psm_req1);
		
		set = true;
	}
}

static void networkHandle(void)
{
    static HalTime_t startConnectTime = 0;

    if(HalNetOnline())
    {
    	setSleepMode();
        if(!OneNetConnected() && HalTimeHasPast(startConnectTime, SECONDS(35)))
        {
            OneNetStartConnect();
            startConnectTime = HalTime();
        }
    }
}

static void startSleep(void)
{
	g_startSleep = true;
	g_startSleepTime = HalTime();
	HalLog("");

	ril_power_saving_mode_setting_rsp_t psm_rsp1;
	opencpu_get_psmparam(&psm_rsp1);
	HalPrint("PSM:%d,%s,%s,%s,%s\n",psm_rsp1.mode,psm_rsp1.req_prdc_rau,
	              psm_rsp1.req_gprs_rdy_tmr,psm_rsp1.req_prdc_tau,psm_rsp1.req_act_time);
}

static void sleepHandle(void)
{
	if(g_startSleep && HalTimeHasPast(g_startSleepTime, SECONDS(15)))
	{
		g_startSleep = false;
		fallSleep();
	}
}

static void valueReport(void)
{
    static HalTime_t lastReportTime = 0;
    if(!g_startSleep && OneNetConnected())
    {
        OneNetDataReport("23.5");
        //lastReportTime = HalTime();//pdMS_TO_TICKS
        startSleep();
    }
}

void APPPoll(void)
{
	valueReport();
	networkHandle();
	OneNetPoll();
	getCSQ();
	sleepHandle();
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


