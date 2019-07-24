#include "Accel.h"
#include "Hal.h"

#define ACCEL_IIC_ADDR 0x1c
#define ACCEL_MMA8451_ID 0x1a
typedef struct
{
    uint8_t reg;
    uint8_t value;
}AccelRegSet_t;

int16_t *AccelReadAxis(void) 
{
    uint8_t value[6] = {0};
    static int16_t axisData[3];

    opencpu_i2c_write_read(ACCEL_IIC_ADDR, MMA8451_OUT_X_MSB, value, sizeof(value));
    axisData[0] = (int16_t)((value[0] << 8) | value[1]);
    axisData[1] = (int16_t)((value[2] << 8) | value[3]);    
    axisData[2] = (int16_t)((value[4] << 8) | value[5]);
    return axisData;
}

void AccelStandby(void)
{
    AccelRegSet_t data;
    data.reg = MMA8451_CTRL_REG1;
    data.value = 0x00;
    opencpu_i2c_write(ACCEL_IIC_ADDR, (uint8_t *)&data, sizeof(AccelRegSet_t));
}

int AccelInit(void)
{
    uint8_t id = 0;
    AccelRegSet_t data;
    
    opencpu_i2c_init();
    opencpu_i2c_set_freq(HAL_I2C_FREQUENCY_100K);
    opencpu_i2c_write_read(ACCEL_IIC_ADDR, MMA8451_WHO_AM_I, &id, 1);
    
    HalLog("id = %02x", id);
    if(id == ACCEL_MMA8451_ID)
    {
        //reset 
        data.reg = MMA8451_CTRL_REG2;
        data.value = 0x40;
        opencpu_i2c_write(ACCEL_IIC_ADDR, (uint8_t *)&data, sizeof(AccelRegSet_t));
        opencpu_delay_ms(100);
        
        data.reg = MMA8451_CTRL_REG1;
        data.value = 0x01;
        opencpu_i2c_write(ACCEL_IIC_ADDR, (uint8_t *)&data, sizeof(AccelRegSet_t));
        opencpu_delay_ms(1);
        
        data.reg = MMA8451_CTRL_REG2;
        data.value = 0x02;
        opencpu_i2c_write(ACCEL_IIC_ADDR, (uint8_t *)&data, sizeof(AccelRegSet_t));
        opencpu_delay_ms(1);

        data.reg = MMA8451_XYZ_DATA_CFG;
        data.value = 0x10;
        opencpu_i2c_write(ACCEL_IIC_ADDR, (uint8_t *)&data, sizeof(AccelRegSet_t));
        return 0;
    }

    return -1;
}

void AccelPoll(void)
{
    
}

