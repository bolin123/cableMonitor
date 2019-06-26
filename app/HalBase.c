#include "Hal.h"

#define GKI_LOG_NAME "emmi"
#define HSL_LOG_NAME "uls"
#define USER_GKI_LOG_PORT SERIAL_PORT_DEV_USB_COM1    //请在这里修改用户GKI log的输出串口，默认为USB COM2
#define USER_HSL_LOG_PORT SERIAL_PORT_DEV_USB_COM2  //请在这里修改用户HSL log的输出串口，默认为USB COM1

/********************************************************************************/
//此函数为opencpu产线模式相关的回调函数，返回1则版本下载到模组后，以AT命令方式启动，需要先执行AT+ATCLOSE命令，之后才会以opencpu方式启动。
//返回0则以opencpu方式启动
//请务必联系技术支持后再确定返回值
//请客户务必保留此函数中所有代码，仅根据需求调整log输出的串口号和返回值。否则可能造成功能紊乱
int get_factory_mode(void)
{
	
	//以下代码是判断模组log口的输出串口号，确保生效的设置和用户的设置一致，防止log口设置错误而干扰用户
	serial_port_dev_t gki_port = -1;
	serial_port_dev_t hsl_port = -1;
	
	opencpu_read_port_config(GKI_LOG_NAME,&gki_port);
	opencpu_read_port_config(HSL_LOG_NAME,&hsl_port);
	
	if( (gki_port != USER_GKI_LOG_PORT) || (hsl_port != USER_HSL_LOG_PORT)) //根据读的结果决定是否写
	{
		opencpu_write_port_config(GKI_LOG_NAME,USER_GKI_LOG_PORT);
		opencpu_write_port_config(HSL_LOG_NAME,USER_HSL_LOG_PORT);
		opencpu_reboot();//因为设置是重启生效，所以设置完必须有reboot函数
	}

	
	return 1;
}

/********************************************************************************/
//此函数为wakeup引脚中断回调函数，在wakein引脚拉低时触发执行
//睡眠状态下测试时，打印函数不会生效，因为系统刚刚恢复，还未初始化uart
void opencpu_wakeup_callback()
{
	HalPrint("opencpu wakeup\n");
}


void opencpu_fota_progress_cb(int current,int total)
{
}

void opencpu_fota_event_cb(int event,int state)
{
}

void opencpu_stack_overflow_hook(xTaskHandle *pxTask, signed portCHAR * pcTaskName)
{
	
}

void vApplicationTickHook( void )
{

}

void opencpu_task_idle_hook(void)
{
	
}

unsigned long opencpu_fota_version_cb(void)
{
	return "V1.0.1";
}



