#include "Hal.h"
#include <stdarg.h>

#define HAL_LOG_UART_NUM HAL_UART_1

unsigned char uart_cmd = 0;

//uart中断回调函数
static void user_uart_callback(hal_uart_callback_event_t status, void *user_data)
{
   char buffer[64];
   char *pbuf;
   pbuf = buffer;
   int temp1;
   if(status == HAL_UART_EVENT_READY_TO_READ)
   {
	   memset(buffer,0,64);
       temp1 = opencpu_uart_receive(HAL_LOG_UART_NUM, pbuf, 64);
	  // opencpu_printf("get:%d\n",temp1);
	   //opencpu_printf("%s",pbuf);
	   uart_cmd = pbuf[0];
   }
}

bool HalNetOnline(void)
{
    return (opencpu_cgact() == 1);
}

void HalReboot(void)
{
    opencpu_reboot();
}

void HalUartPrintf(const char *str, ...)
{
    static unsigned char s[600]; //This needs to be large enough to store the string TODO Change magic number
	int i;
	unsigned char *p;
    va_list args;
    int str_len;

    if ((str == NULL) || (strlen(str) == 0))
    {
        return;
    }
    va_start (args, str);
    str_len = (unsigned int)vsprintf((char*)s, str, args);
    va_end (args);
    opencpu_uart_send(HAL_LOG_UART_NUM, s, str_len);
}

static void logUartInit(void)
{
	opencpu_uart_open(HAL_LOG_UART_NUM, HAL_UART_BAUDRATE_115200, user_uart_callback);
}

void HalPoll(void)
{
}

void HalInitialize(void)
{
	logUartInit();
}

