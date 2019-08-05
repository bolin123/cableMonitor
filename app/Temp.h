#ifndef TEMP_H
#define TEMP_H

// 传感器工作的精度
#define      ACCURACY_MODE_9  

// 温度精度宏 
#define      ACCURACY_9            0x1F    
#define      ACCURACY_10           0x3F        
#define      ACCURACY_11           0x5F        
#define      ACCURACY_12           0x7F     

// 温度转换实际时间
#define      CONVERT_T_9           96   
#define      CONVERT_T_10          188   
#define      CONVERT_T_11          376   
#define      CONVERT_T_12          760   


#if defined(ACCURACY_MODE_9)
    #define  ACCURACY             ACCURACY_9
	#define  CONVERT_T            CONVERT_T_9
#elif defined(ACCURACY_MODE_10)
    #define  ACCURACY             ACCURACY_10
	#define  CONVERT_T            CONVERT_T_10
#elif defined(ACCURACY_MODE_11)
    #define  ACCURACY             ACCURACY_11
	#define  CONVERT_T            CONVERT_T_11
#elif defined(ACCURACY_MODE_12)
    #define  ACCURACY             ACCURACY_12
	#define  CONVERT_T            CONVERT_T_12
#else
#endif


unsigned short TemperatureGetValue(void);
float TemperatureValueExchange(unsigned short temp);
void TemperatureInit(void);
//void TemperaturePoll(void);
#endif

