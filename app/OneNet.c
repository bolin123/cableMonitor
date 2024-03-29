#include "OneNet.h"

#define ONE_NET_IP "183.230.40.39"

#ifdef HAL_DEVICE_TYPE_TEMPERATURE
#define ONE_NET_OBJID_MAIN  3303 //Temperature
#define ONE_NET_RESOURCEID_SENSOR_VALUE 5700
#else
#define ONE_NET_OBJID_MAIN  3313 //Accelerometer
#define ONE_NET_RESOURCEID_SENSOR_VALUE 5700
#endif
#define ONE_NET_OBJID_FREQUENCY  3310
#define ONE_NET_RESOURCEID_FREQUENCY 5825
#define ONE_NET_CONNECT_LIFETIME (24*60*60)

typedef enum
{
	ONENET_CONNECT_STATUS_UNKOWN = 0,
	ONENET_CONNECT_STATUS_CONNECT,
	ONENET_CONNECT_STATUS_DISCONNECT,
}OneNetConnectStatus_t;

static bool g_onenetConnected = false;
static HalTime_t g_updateTime;
//static OneNetConnectStatus_t g_connStatus = ONENET_CONNECT_STATUS_UNKOWN;

static char *num2String(uint32_t num)
{
	static char buff[8];
	
	buff[0] = '\0';
	sprintf(buff, "%d", num);
	return buff;
}

static void oneNetEventCallback(int event)
{
	HalPrint("event:%d\n",event);
	switch(event)
	{
		case CIS_EVENT_REG_SUCCESS: 
		{
			g_updateTime = HalTime();
			HalPrint("onenet register done!\n");
			break;
		}
		case CIS_EVENT_UNREG_DONE:
		{
			g_onenetConnected = false;
			//g_connStatus = ONENET_CONNECT_STATUS_DISCONNECT;
			HalPrint("onenet register failed\n");
			break;
		}
		case CIS_EVENT_FIRMWARE_TRIGGER:
		{
			HalPrint("new firmware is online\n");
			break;
		}
		case CIS_EVENT_CONNECT_SUCCESS:
			//g_connStatus = ONENET_CONNECT_STATUS_CONNECT;
			break;
		case CIS_EVENT_CONNECT_FAILED:
		case CIS_EVENT_REG_FAILED:
		case CIS_EVENT_REG_TIMEOUT:
		case CIS_EVENT_LIFETIME_TIMEOUT:
			//g_connStatus = ONENET_CONNECT_STATUS_DISCONNECT;
			g_onenetConnected = false;
			break;
		default:
     		break;
	}
}

static void oneNetNotifyCallback(int ack_id)
{
    //TODO
    //...
    HalPrint("ack_id:%d\n",ack_id);
}

static void oneNetReadCallback(int mid, int objid, int insid, int resid)
{
    //TODO
    //...
    //根据传入的objid_insid_resid决定需上传的数据
    HalPrint("read:%d %d\n", objid, resid);
    if((objid == 3200) && (resid == 5750))
    {
        opencpu_onenet_read(mid, objid, insid, resid, 1, "test read!", 1);//返回表示该操作结果数据,示例资源类型为
    }
    else
    {
        opencpu_onenet_result(mid, RESULT_400_BADREQUEST, 0);//返回表示该操作结果错误
    }
}

static void oneNetWriteCallback(int mid, int objid,int insid, int resid, int type, int flag, int len, char *data)
{
    //TODO
    //...
    HalPrint("write:%d %s\n",len,data);
	if(objid == ONE_NET_OBJID_FREQUENCY && resid == ONE_NET_RESOURCEID_FREQUENCY)
	{
		HalIntervalSet((uint16_t)atoi(data));
	}
    opencpu_onenet_result(mid, RESULT_204_CHANGED, 0);//操作正确完成返回204
}

static void oneNetExecuteCallback(int mid, int objid, int insid, int resid, int len, char *data)
{
    //TODO
    //...
    HalPrint("write:%d %s\n",len,data);
    opencpu_onenet_result(mid, RESULT_204_CHANGED, 0);//操作正确完成返回204
}

static void oneNetObserveCallback(int mid, int observe, int objid, int insid, int resid)
{
    //TODO
    //...
    HalPrint("%d_%d_%d: %d\n", objid, insid, resid, observe);//对应的objid被observe后方可notify上报
    opencpu_onenet_result(mid, RESULT_205_CONTENT, 1);//操作正确完成返回204
    
	g_onenetConnected = true;
}

static void oneNetParameterCallback(int mid,int objid, int insid, int resid, int len, char *parameter)
{
    //TODO
    //...
    opencpu_onenet_result(mid, RESULT_204_CHANGED, 0);//操作正确完成返回204
}
/*
static void onenetStatusQuery(void)
{
	int result;
	static HalTime_t queryTime;
	if(g_connStatus == ONENET_CONNECT_STATUS_UNKOWN && HalNetOnline())
	{
		if(HalTimeHasPast(queryTime, SECONDS(20)))
		{
			result = opencpu_onenet_is_open();
			if(result < 0)
			{
				// TODO: error
				HalLog("err: %d", result);
			}
			else 
			{
				if(result == 600)
				{
					g_connStatus = ONENET_CONNECT_STATUS_DISCONNECT;
				}
			}
			queryTime = HalTime();
		}
	}
}
*/
bool OneNetConnected(void)
{
    //return g_connStatus == ONENET_CONNECT_STATUS_CONNECT;
    return g_onenetConnected;
}

void OneNetClose(void)
{
    opencpu_onenet_close(2);
}

void OneNetDataReport(char *data)
{
//#ifdef HAL_DEVICE_TYPE_TEMPERATURE
    HalLog("%s", data);
    opencpu_onenet_notify(ONE_NET_OBJID_MAIN, 
                        0, ONE_NET_RESOURCEID_SENSOR_VALUE, 
                        4, data, 
                        1, -1, 0);

	uint16_t interval = HalIntervalGet();
	char buff[6];
	sprintf(buff, "%d", interval);
	opencpu_onenet_notify(ONE_NET_OBJID_FREQUENCY, 
                        0, ONE_NET_RESOURCEID_FREQUENCY, 
                        1, buff, 
                        1, -1, 0);
//#else
//#endif
}

int OneNetStartConnect(void)
{
    HalLog("");
    OneNetCreate();
    //timeout 30, lifetime 3600s.
    if(opencpu_onenet_open(30, ONE_NET_CONNECT_LIFETIME) == 0)
    {
        return 0;
    }
    return -1;
}

static void connectUpdate(void)
{
    //opencpu_onenet_update(unsigned int lifetime, int flag);
}

void OneNetCreate(void)
{
    cot_cb_t callback;
	//TickType_t lastTime;
	
	callback.onRead    = oneNetReadCallback;
	callback.onWrite   = oneNetWriteCallback;
	callback.onExec    = oneNetExecuteCallback;
	callback.onObserve = oneNetObserveCallback;
	callback.onParams  = oneNetParameterCallback;
	callback.onEvent   = oneNetEventCallback;
	callback.onNotify  = oneNetNotifyCallback;
	callback.onDiscover = NULL;

	HalLog("");
	opencpu_onenet_init();//初始化任务
	opencpu_onenet_create(ONE_NET_IP, 1, &callback);//创建设备
	//#ifdef HAL_DEVICE_TYPE_TEMPERATURE
	HalLog("add obj");
   	opencpu_onenet_add_obj(ONE_NET_OBJID_MAIN, 1, "1", 0, 0);
    opencpu_onenet_discover(ONE_NET_OBJID_MAIN, 4, num2String(ONE_NET_RESOURCEID_SENSOR_VALUE));//杩?炲皢?ㄥ埌鐨勮祫婧?楄?
	opencpu_onenet_add_obj(ONE_NET_OBJID_FREQUENCY, 1, "1", 0, 0);
    opencpu_onenet_discover(ONE_NET_OBJID_FREQUENCY, 4, num2String(ONE_NET_RESOURCEID_FREQUENCY));//杩?炲皢?ㄥ埌鐨勮祫婧?楄?
   // #else
    //#endif

}

void OneNetInitialize(void)
{

}

void OneNetPoll(void)
{
	//onenetStatusQuery();
}

