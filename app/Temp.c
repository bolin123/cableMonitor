#include "Temp.h"
#include "Hal.h"

#define TEMP_18B20_DQ_PIN HAL_GPIO_20 //pd11

#define TEMP_18B20_DQ_INPUT() hal_gpio_set_direction(TEMP_18B20_DQ_PIN, HAL_GPIO_DIRECTION_INPUT)
#define TEMP_18B20_DQ_OUTPUT() hal_gpio_set_direction(TEMP_18B20_DQ_PIN, HAL_GPIO_DIRECTION_OUTPUT)
#define TEMP_18B20_DQ_SET_LEVEL(x) hal_gpio_set_output(TEMP_18B20_DQ_PIN, x)
//#define TEMP_18B20_DQ_GET_LEVEL() HalGPIOGetLevel(TEMP_18B20_DQ_PIN)
 
 static int TEMP_18B20_DQ_GET_LEVEL(void)
{
	hal_gpio_data_t iodata;
	hal_gpio_get_input(TEMP_18B20_DQ_PIN, &iodata);	
	return iodata;
}
 
static int tempReset(void)
{
    uint8_t retry = 0;
    TEMP_18B20_DQ_OUTPUT();             //SET PG11 OUTPUT
    TEMP_18B20_DQ_SET_LEVEL(0);         //拉低DQ
    opencpu_delay_us(500);                     //拉低 > 480us 
    TEMP_18B20_DQ_INPUT();              //SET PG11 INPUT    
    opencpu_delay_us(5);
    while (TEMP_18B20_DQ_GET_LEVEL() && retry < 60)//wait 15~60us
    {
        retry++;
        opencpu_delay_us(1);
    }       
    if(retry >= 65)
    {
        return -1;
    }

    retry=0;
    while (!TEMP_18B20_DQ_GET_LEVEL() && retry < 250)//wait 60~240
    {
        retry++;
        opencpu_delay_us(1);
    }
    if(retry >= 250)
    {
        return -1;
    }            
    opencpu_delay_us(5);
    return 0;
}
 /*
 static uint8_t readBit(void)
 {
     uint8_t data;
     TEMP_18B20_DQ_OUTPUT();        //SET PG11 OUTPUT
     TEMP_18B20_DQ_SET_LEVEL(0); 
     HalWaitUs(2);
     //TEMP_18B20_DQ_SET_LEVEL(1); 
     TEMP_18B20_DQ_INPUT();        //SET PG11 INPUT
     HalWaitUs(3); 
     data = TEMP_18B20_DQ_GET_LEVEL();
     HalWaitUs(56);           
     return data;
 }
 */
 static uint8_t tempReadByte(void)
 {
     uint8_t mask;
     uint8_t data = 0x00;
     /*
     * 所有的读时隙必须至少有60us的持续时间
     * 相邻两个读时隙必须要有最少1us的恢复时间
     * 所有的读时隙都由拉低总线，持续至少1us后再释放总线
     */
#if 1
     for(mask = 0x01; mask != 0; mask <<= 1) 
     {
         TEMP_18B20_DQ_OUTPUT();
         TEMP_18B20_DQ_SET_LEVEL(0); 
         opencpu_delay_us(10);  //t1 > 1us, 至少1us后再释放总线(9.6)
         TEMP_18B20_DQ_INPUT();
         //master sample init, do nothing
         opencpu_delay_us(18);  //t2, t1 + t2 <= 15us (18.6)
 
         //等待数据稳定
         //HalWaitUs(2); //t3
         if (TEMP_18B20_DQ_GET_LEVEL())
         {
             data |= mask;
         }
         else
         {
             data  &= ~mask;
         }
         opencpu_delay_us(46); //t4 + t3 >= 45us 
     }
#else
     for(mask = 0x01; mask != 0; mask <<= 1) 
     {
         TEMP_18B20_DQ_OUTPUT();
         TEMP_18B20_DQ_SET_LEVEL(1); 
         HalWaitUs(5);
         TEMP_18B20_DQ_SET_LEVEL(0); 
         HalWaitUs(5);
         TEMP_18B20_DQ_SET_LEVEL(1); 
         HalWaitUs(5);
         TEMP_18B20_DQ_INPUT();
         HalWaitUs(1); 
         if (TEMP_18B20_DQ_GET_LEVEL())
             data |= mask;
         else    
             data  &= ~mask;
         TEMP_18B20_DQ_OUTPUT();
         TEMP_18B20_DQ_SET_LEVEL(1); 
         HalWaitUs(56);
     }
#endif
     return data;
 }
 
 static void tempWriteByte(uint8_t cmd)
 {             
     uint8_t i;
     /*
     * 写时隙必须有最少60us的持续时间(60~120)
     * 写1时隙，在拉低总线后主机必须在15μs内释放总线|_<15_|
     * 写0时隙，在拉低总线后主机必须继续拉低总线以满足时隙持续时间的要求(至少60μs)
     * 相邻两个写时隙必须要有最少1us的恢复时间
     */
     TEMP_18B20_DQ_OUTPUT();
     for (i = 0; i < 8; i++) 
     {
         TEMP_18B20_DQ_SET_LEVEL(1); //idle
         opencpu_delay_us(8);               //最少1us的恢复时间
         TEMP_18B20_DQ_SET_LEVEL(0); //start
         opencpu_delay_us(8);              //t1 < 15us    
         TEMP_18B20_DQ_SET_LEVEL(cmd & 0x01);
         opencpu_delay_us(64);              //t2, t1+t2 >= 60us,
         cmd = cmd >> 1;
     }
     TEMP_18B20_DQ_SET_LEVEL(1); //idle
 }
 
 static uint8_t crc1Byte(uint8_t one_byte)      
 {      
    uint8_t i,crc_one_byte;       
    crc_one_byte=0;   
    for(i = 0; i < 8; i++)      
    {      
         if(((crc_one_byte^one_byte)&0x01))      
         {      
            crc_one_byte^=0x18;       
            crc_one_byte>>=1;      
            crc_one_byte|=0x80;      
         }            
         else
         {
             crc_one_byte>>=1;
         }
         one_byte>>=1;            
     }     
     return crc_one_byte;     
 }  
 
 static uint8_t crc(uint8_t *p, uint8_t len)    
 {    
     uint8_t crc=0;    
     while(len--)  
     {    
         crc = crc1Byte(crc^(*p++));    
     }    
     return crc;    
 }
 
 uint16_t TemperatureGetValue(void)
 {
#if 1
     uint8_t tl;
     uint8_t i;
     uint16_t value = 0;
     uint8_t data[9];
 //    int ret1, ret2;
     
     tempReset();
     tempWriteByte(0xcc);        // skip rom
     tempWriteByte(0x44);        // convert
     opencpu_delay_us(CONVERT_T);
     //HalWaitMs(1);
     tempReset();
     tempWriteByte(0xcc);        // skip rom
     tempWriteByte(0xbe);        // convert  
     /*
     tl = tempReadByte();         // LSB   
     value = tempReadByte();         // MSB  
     value = (value << 8) + tl;
     */
     for(i = 0; i < 9; i++)
     {
         data[i] = tempReadByte();
     }
     if(crc(data, 8) == data[8])
     {
         value = data[1];
         value <<= 8;
         value |= data[0];
         
         return value;
     }
     HalLog("temp crc err!\n");
     return 0;
    #else
     uint8_t tl;
     //tempReset();
     //tempReadByte();
     //tempWriteByte(0xcc);
     //tempWriteByte(0xbe);        // convert            
     //tl = tempReadByte();
     return 0;
     
#endif
 }
 
 float TemperatureValueExchange(uint16_t temp)
 {
     float dat;
     
     // 获取温度的实际数值，不包含符号位
     dat = (temp >> 4) & 0x7F;                         //提取整数部分
     dat += (float)(temp&0x0F) / 16;                 //提取小数部分
     // 判断温度的符号
     if (0 != (temp & 0xF800))   //判断符号为，全为1表示零下温度值
     {                      
         return -dat;
     } 
     
     return dat;
 }


void TemperatureInit(void)
{
	int ret;
	hal_gpio_init(TEMP_18B20_DQ_PIN);
	hal_pinmux_set_function(TEMP_18B20_DQ_PIN, HAL_GPIO_20_GPIO20);
	hal_gpio_pull_up(TEMP_18B20_DQ_PIN);
 
#if 1
    ret = tempReset();
    tempWriteByte(0xCC);                        //跳过ROM
    // 设置配置寄存器，精确到9Bit即0.5C'
    tempWriteByte(0x4E);                        //设置暂存器指令
    tempWriteByte(0xFF);                        //TH
    tempWriteByte(0xFF);                        //TL
    tempWriteByte(ACCURACY);                        //config寄存器

    tempWriteByte(0x44);                        //启动一次温度转换
	HalLog("ret = %d", ret);
    #endif
}

#if 0
#define TEMP_18B20_DQ_PIN HAL_GPIO_1 //pd11
#define TEMP_18B20_DQ_INPUT() hal_gpio_set_direction(TEMP_18B20_DQ_PIN, HAL_GPIO_DIRECTION_INPUT)
#define TEMP_18B20_DQ_OUTPUT() hal_gpio_set_direction(TEMP_18B20_DQ_PIN, HAL_GPIO_DIRECTION_OUTPUT)
#define TEMP_18B20_DQ_SET_LEVEL(x) hal_gpio_set_output(TEMP_18B20_DQ_PIN, x)
//#define TEMP_18B20_DQ_GET_LEVEL() HalGPIOGetLevel(TEMP_18B20_DQ_PIN)
 
 static int TEMP_18B20_DQ_GET_LEVEL(void)
{
	hal_gpio_data_t iodata;
	hal_gpio_get_input(TEMP_18B20_DQ_PIN, &iodata);	
	return iodata;
}
 static int tempReset(void)
 {
	 uint8_t retry = 0;
	 TEMP_18B20_DQ_OUTPUT();		 //SET PG11 OUTPUT
	 //TEMP_18B20_DQ_SET_LEVEL(1);		   //DQ=1 
	 //opencpu_opencpu_delay_us(700);
	 TEMP_18B20_DQ_SET_LEVEL(0);		 //拉低DQ
	 opencpu_opencpu_delay_us(550);			//拉低480us
	 TEMP_18B20_DQ_SET_LEVEL(1);		 //DQ=1 
	 opencpu_opencpu_delay_us(1);			   //15US
 
	 TEMP_18B20_DQ_INPUT(); 	   //SET PG11 INPUT 		
	 while (TEMP_18B20_DQ_GET_LEVEL() && retry < 200)
	 {
		 retry++;
		 opencpu_opencpu_delay_us(2);
	 }		 
	 if(retry >= 200)
	 {
		 return -1;
	 }
 
	 retry=0;
	 while (!TEMP_18B20_DQ_GET_LEVEL() && retry < 240)
	 {
		 retry++;
		 opencpu_opencpu_delay_us(2);
	 }
	 if(retry >= 240)
	 {
		 return -1;
	 }			  
	 return 0;
 }
 /*
 static uint8_t readBit(void)
 {
	 uint8_t data;
	 TEMP_18B20_DQ_OUTPUT();		//SET PG11 OUTPUT
	 TEMP_18B20_DQ_SET_LEVEL(0); 
	 opencpu_opencpu_delay_us(2);
	 //TEMP_18B20_DQ_SET_LEVEL(1); 
	 TEMP_18B20_DQ_INPUT(); 	   //SET PG11 INPUT
	 opencpu_opencpu_delay_us(3); 
	 data = TEMP_18B20_DQ_GET_LEVEL();
	 opencpu_opencpu_delay_us(56); 		  
	 return data;
 }
 */
 static uint8_t tempReadByte(void)
 {
	 uint8_t mask;
	 uint8_t data = 0x00;
 
	 for(mask = 0x01; mask != 0; mask <<= 1) 
	 {
		 TEMP_18B20_DQ_OUTPUT();
		 TEMP_18B20_DQ_SET_LEVEL(0); 
		 opencpu_opencpu_delay_us(2);
		 TEMP_18B20_DQ_INPUT();
		 opencpu_opencpu_delay_us(3); 
		 if (TEMP_18B20_DQ_GET_LEVEL())
			 data |= mask;
		 else	 
			 data  &= ~mask;
		 opencpu_opencpu_delay_us(56);
	 }
	 return data;
 }
 
 static void tempWriteByte(uint8_t cmd)
 {			   
	 uint8_t i;
	 TEMP_18B20_DQ_OUTPUT();		//SET PG11 OUTPUT;
	 for (i = 0; i < 8; i++) 
	 {
		 TEMP_18B20_DQ_SET_LEVEL(1);
		 opencpu_opencpu_delay_us(1);	 
		 TEMP_18B20_DQ_SET_LEVEL(0);		// Write 1
		 opencpu_opencpu_delay_us(2);							  
		 TEMP_18B20_DQ_SET_LEVEL(cmd & 0x01);
		 opencpu_opencpu_delay_us(58); 
		 cmd = cmd >> 1;
	 }
	 TEMP_18B20_DQ_SET_LEVEL(1);
 }
 
 uint16_t TemperatureGetValue(void)
 {
	 int ret1, ret2;
	 uint8_t tl;
	 uint16_t value = 0;
	 
	 ret1 = tempReset();
	 tempWriteByte(0xcc);		 // skip rom
	 tempWriteByte(0x44);		 // convert
	 opencpu_opencpu_delay_us(CONVERT_T);
	 ret2 = tempReset();
	 tempWriteByte(0xcc);		 // skip rom
	 tempWriteByte(0xbe);		 // convert 		   
	 tl = tempReadByte();		  // LSB   
	 value = tempReadByte();		 // MSB  
	 value = (value << 8) + tl;
	 //Syslog("value = %d", value);
	 HalPrint("ret1 = %d, ret2 = %d\n", ret1, ret2);
	 return value;
 }
 
 float TemperatureValueExchange(uint16_t temp)
 {
	 float dat;
	 
	 // 获取温度的实际数值，不包含符号位
	 dat = (temp >> 4) & 0x7F;						   //提取整数部分
	 dat += (float)(temp&0x0F) / 16;				 //提取小数部分
	 // 判断温度的符号
	 if (0 != (temp & 0xF800))	 //判断符号为，全为1表示零下温度值
	 {						
		 return -dat;
	 } 
	 
	 return dat;
 }
 
 void TemperatureInit(void)
 {
	hal_gpio_init(TEMP_18B20_DQ_PIN);
	hal_pinmux_set_function(TEMP_18B20_DQ_PIN, HAL_GPIO_1_GPIO1);
	hal_gpio_pull_up(TEMP_18B20_DQ_PIN);
	TEMP_18B20_DQ_OUTPUT();
	TEMP_18B20_DQ_SET_LEVEL(0);
	tempReset();
	tempWriteByte(0xCC);						 //跳过ROM
	// 设置配置寄存器，精确到9Bit即0.5C'
	tempWriteByte(0x4E);						 //设置暂存器指令
	tempWriteByte(0xFF);						 //TH
	tempWriteByte(0xFF);						 //TL
	tempWriteByte(ACCURACY);						 //config寄存器

	tempWriteByte(0x44);						 //启动一次温度转换
 }

void TemperaturePoll(void)
{
}
#endif
 #if 0

 
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
		opencpu_opencpu_delay_us(650);
		DQ_SET_LEVEL(1);
		opencpu_opencpu_delay_us(50);
		DQ_DIRCT_INPUT();
		opencpu_opencpu_delay_us(650);
		if(DQ_GET_LEVEL() == 0)
		{
			return 0;
		}
	}
	return -1;
#endif
	int count = 0;
	int ret = -1;
	DQ_DIRCT_OUTPUT();
	DQ_SET_LEVEL(0); //单片机拉低总线
	opencpu_opencpu_delay_us(750); //精确延时，维持至少480us
	DQ_SET_LEVEL(1); //释放总线，即拉高了总线
	opencpu_opencpu_delay_us(15); //此处延时有足够，确保能让DS18B20发出存在脉冲。
	DQ_DIRCT_INPUT();
	do
	{
		if(DQ_GET_LEVEL() == 0)
		{
			ret = 0;
			break;
		}
		count++;
		opencpu_opencpu_delay_us(10);
	}while(count < 30);

	opencpu_opencpu_delay_us(100);
	DQ_DIRCT_OUTPUT();
	DQ_SET_LEVEL(1); //单片机拉低总线
	return ret;
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
        opencpu_opencpu_delay_us(2);                //延时等待
        DQ_SET_LEVEL(1);                     //准备接收
        opencpu_opencpu_delay_us(2);                //接收延时
        DQ_DIRCT_INPUT();
		//opencpu_opencpu_delay_us(5);
        if (DQ_GET_LEVEL()) 
		{
			dat |= 0x80;        //读取数据
		}
        opencpu_opencpu_delay_us(60);               //等待时间片结束
        DQ_DIRCT_OUTPUT();
		DQ_SET_LEVEL(1);
		opencpu_opencpu_delay_us(2);
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
        opencpu_opencpu_delay_us(2);                //延时等待
        DQ_SET_LEVEL(1);                     //开始时间片
        opencpu_opencpu_delay_us(2);                //延时等待
        DQ_SET_LEVEL(value & 0x01);
		value = value >> 1;
        opencpu_opencpu_delay_us(55);               //等待时间片结束
        DQ_SET_LEVEL(1);                     //恢复数据线
        opencpu_opencpu_delay_us(2);                //恢复延时
    }
}

short TempGetValue(void)
{
	uint8_t tpl;
	int result;
	uint16_t value;
	uint16_t delay = 0;
	result = DS18B20Reset();				//设备复位
	HalLog("reset result = %d", result);
	DS18B20WriteByte(0xCC);		//跳过ROM命令
	DS18B20WriteByte(0x44);		//开始转换命令
	#if 0
	DQ_DIRCT_INPUT();
	while (!DQ_GET_LEVEL())			    //等待转换完成
	{
		delay++;
		opencpu_opencpu_delay_us(10);
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
#endif
