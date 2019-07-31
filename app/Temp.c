#include "Temp.h"
#include "Hal.h"

#define DQ_IO_PIN HAL_GPIO_1

#define DQ_DIRCT_OUTPUT() hal_gpio_set_direction(DQ_IO_PIN, HAL_GPIO_DIRECTION_OUTPUT); hal_gpio_pull_up(DQ_IO_PIN)
#define DQ_DIRCT_INPUT() hal_gpio_set_direction(DQ_IO_PIN, HAL_GPIO_DIRECTION_INPUT)
#define DQ_SET_LEVEL(x) hal_gpio_set_output(DQ_IO_PIN, x)
//#define DQ_GET_LEVEL() HalGPIOGetLevel(DQ_GPIO_NUM)

static int DQ_GET_LEVEL(void)
{
	hal_gpio_data_t iodata;
	hal_gpio_get_input(DQ_IO_PIN, &iodata);	
	if(iodata == HAL_GPIO_DATA_HIGH)
	{
		return 1;
	}
	return 0;
}
 
/**************************************
复位DS18B20,并检测设备是否存在
**************************************/
static int DS18B20Reset(void)
{
#if 0
	uint8_t i;
	
	for(i = 0; i < 10; i++)
	{
		DQ_DIRCT_OUTPUT();
		DQ_SET_LEVEL(0);
		opencpu_delay_us(650);
		DQ_SET_LEVEL(1);
		opencpu_delay_us(50);
		DQ_DIRCT_INPUT();
		opencpu_delay_us(650);
		if(DQ_GET_LEVEL() == 0)
		{
			return 0;
		}
	}
	return -1;
#endif
	DQ_DIRCT_OUTPUT();
	DQ_SET_LEVEL(1); //DQ复位，不要也可行。
	opencpu_delay_us(7); //稍做延时
	DQ_SET_LEVEL(0); //单片机拉低总线
	opencpu_delay_us(550); //精确延时，维持至少480us
	DQ_SET_LEVEL(1); //释放总线，即拉高了总线
	opencpu_delay_us(250); //此处延时有足够，确保能让DS18B20发出存在脉冲。
	return 0;
}
 
/**************************************
从DS18B20读1字节数据
**************************************/
static uint8_t DS18B20ReadByte(void)
{
    uint8_t i;
    uint8_t dat = 0;
 
    for (i=0; i<8; i++)             //8位计数器
    {
    	DQ_DIRCT_OUTPUT();
        dat >>= 1;
        DQ_SET_LEVEL(0);                     //开始时间片
        opencpu_delay_us(2);                //延时等待
        DQ_SET_LEVEL(1);                     //准备接收
        opencpu_delay_us(7);                //接收延时
        DQ_DIRCT_INPUT();
		//opencpu_delay_us(5);
        if (DQ_GET_LEVEL()) 
		{
			dat |= 0x80;        //读取数据
		}
        opencpu_delay_us(60);               //等待时间片结束
    }
 
    return dat;
}
 
/**************************************
向DS18B20写1字节数据
**************************************/
static void DS18B20WriteByte(uint8_t dat)
{
    char i;
	uint8_t value = dat;
 
	DQ_DIRCT_OUTPUT();
    for (i=0; i<8; i++)             //8位计数器
    {
        DQ_SET_LEVEL(0);                     //开始时间片
        opencpu_delay_us(2);                //延时等待
        DQ_SET_LEVEL(value & 0x01);
		value = value >> 1;
        opencpu_delay_us(60);               //等待时间片结束
        DQ_SET_LEVEL(1);                     //恢复数据线
        opencpu_delay_us(5);                //恢复延时
    }
}

short TempGetValue(void)
{
	uint8_t tpl;	
	uint16_t value;
	uint16_t delay = 0;
	DS18B20Reset();				//设备复位
	DS18B20WriteByte(0xCC);		//跳过ROM命令
	DS18B20WriteByte(0x44);		//开始转换命令
	#if 0
	DQ_DIRCT_INPUT();
	while (!DQ_GET_LEVEL())			    //等待转换完成
	{
		delay++;
		opencpu_delay_us(10);
		if(delay > 100)
		{
			return 0;
		}
	}
	#endif
	DS18B20Reset();				//设备复位
	DS18B20WriteByte(0xCC);		//跳过ROM命令
	DS18B20WriteByte(0xBE);		//读暂存存储器命令
	tpl = DS18B20ReadByte();		//读温度低字节
	value = DS18B20ReadByte();		//读温度高字节
	return ((value << 8) + tpl);
}

void TempInit(void)
{
	hal_gpio_init(DQ_IO_PIN);
	hal_pinmux_set_function(DQ_IO_PIN, HAL_GPIO_1_GPIO1);
}

void TempPoll(void)
{
}

