#include "Temperature.h"
#include "Hal.h"

#define TEMPERATURE_IIC_ADDR 0x48

void TemperatrueWakeup(void)
{
}

void TemperatureSleep(void)
{
}


float TemperatureGetValue(void)
{
#if 1
    uint8_t data[3];
    int16_t value;
    float tmp;

    data[0] = 0x01;
    data[1] = 0xE7;//one-shot
    opencpu_i2c_write(TEMPERATURE_IIC_ADDR, data, 2);
    opencpu_delay_ms(230);
    opencpu_i2c_write_read(TEMPERATURE_IIC_ADDR, 0x00, data, 2);
    HalLog("d1=%02x d2=%02x", data[0], data[1]);
    value = data[0];
    value = (value<<8) + data[1];
    HalPrint("%x\n", value);
    tmp = (float)value / 256.0;
    //HalPrint("%f\n", tmp);
    return tmp;
#else
    float value = 1.2 * 3.0;
    float temp = 123.11;
    char str[16] = "";
    sprintf(str, "temp = %f", temp);
    HalLog("%s", str);
    HalLog("direct %f", temp);
    HalLog("%f", value);
    return 0;
#endif
    
}

int TemperatureInit(void)
{
    uint8_t data[2];
    uint8_t config;
    opencpu_i2c_init();
    opencpu_i2c_set_freq(HAL_I2C_FREQUENCY_50K);

    data[0] = 0x01;
    data[1] = 0x67;//12 Bits (0.0625°C) ,220ms,Shutdown Mode
    opencpu_i2c_write(TEMPERATURE_IIC_ADDR, data, 2);
    opencpu_i2c_write_read(TEMPERATURE_IIC_ADDR, 0x01, &config, 1);

    HalLog("config = %02x", config);
    return 0;
}

