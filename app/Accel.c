#include "Accel.h"
#include "Hal.h"

#define PI					  3.14159
#define SENSITIVITY_2G		  4096

#define ACCEL_IIC_ADDR 0x1c
#define ACCEL_MMA8451_ID 0x1a
typedef struct
{
    uint8_t reg;
    uint8_t value;
}AccelRegSet_t;

float AccelGetAngle(void)
{
    unsigned char AccData[6];
    short Xout_14_bit, Yout_14_bit, Zout_14_bit;
    float Xout_g, Yout_g, Zout_g, Pitch, Roll;
    char DataReady;
    char Xoffset, Yoffset, Zoffset;
    
//    I2C_ReadMultiRegisters(MMA845x_I2C_ADDRESS, OUT_X_MSB_REG, 6, AccData);     // Read data output registers 0x01-0x06 
    opencpu_i2c_write_read(ACCEL_IIC_ADDR, MMA8451_OUT_X_MSB, AccData, sizeof(AccData));    
    Xout_14_bit = ((short) (AccData[0]<<8 | AccData[1])) >> 2;      // Compute 14-bit X-axis output value
    Yout_14_bit = ((short) (AccData[2]<<8 | AccData[3])) >> 2;      // Compute 14-bit Y-axis output value
    Zout_14_bit = ((short) (AccData[4]<<8 | AccData[5])) >> 2;      // Compute 14-bit Z-axis output value
    
    Xout_g = ((float) Xout_14_bit) / SENSITIVITY_2G;        // Compute X-axis output value in g's
    Yout_g = ((float) Yout_14_bit) / SENSITIVITY_2G;        // Compute Y-axis output value in g's
    Zout_g = ((float) Zout_14_bit) / SENSITIVITY_2G;        // Compute Z-axis output value in g's
    
    Pitch = atan2 (-Xout_g, sqrt (Yout_g*Yout_g + Zout_g*Zout_g)) * 180 / PI;       // Equation 37 in the AN3461
    if (Zout_g > 0)                                                                 // Equation 38 in the AN3461
        Roll = atan2 (Yout_g, sqrt (0.01*Xout_g*Xout_g + Zout_g*Zout_g)) * 180 / PI;
    else
        Roll = atan2 (Yout_g, -sqrt (0.01*Xout_g*Xout_g + Zout_g*Zout_g)) * 180 / PI;

    int16_t pvalue, rvalue;
    pvalue = (int16_t)(Pitch * 10);
    rvalue = (int16_t)(Roll * 10);
    HalPrint("pitch = %d.%d\n", (int16_t)(pvalue / 10), (uint16_t)pvalue % 10);
    HalPrint("roll = %d.%d\n", (int16_t)(rvalue / 10), (uint16_t)rvalue % 10);
    return Pitch;

}


int16_t *AccelReadAxis(void) 
{
    uint8_t value[6] = {0};
    static int16_t axisData[3];
#if 1

    opencpu_i2c_write_read(ACCEL_IIC_ADDR, MMA8451_OUT_X_MSB, value, sizeof(value));
    axisData[0] = (int16_t)((value[0] << 8) | value[1]);
    axisData[1] = (int16_t)((value[2] << 8) | value[3]);    
    axisData[2] = (int16_t)((value[4] << 8) | value[5]);
    axisData[0] >>= 4;
    axisData[1] >>= 4;
    axisData[2] >>= 4;
#endif
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
#if 1
    uint8_t id[3] = {0};
    uint8_t value = 0;
    AccelRegSet_t data;
    opencpu_i2c_init();
    opencpu_i2c_set_freq(HAL_I2C_FREQUENCY_50K);
	#if 0
	data.reg = MMA8451_CTRL_REG1;
    data.value = 0x01;
	opencpu_i2c_write(ACCEL_IIC_ADDR, (uint8_t *)&data, sizeof(AccelRegSet_t));
    opencpu_delay_ms(100);
	data.reg = MMA8451_CTRL_REG2;
    data.value = 0x02;
    opencpu_i2c_write(ACCEL_IIC_ADDR, (uint8_t *)&data, sizeof(AccelRegSet_t));
    opencpu_delay_ms(100);
	data.reg = MMA8451_XYZ_DATA_CFG;
    data.value = 0x10;
    opencpu_i2c_write(ACCEL_IIC_ADDR, (uint8_t *)&data, sizeof(AccelRegSet_t));
    opencpu_i2c_write_read(ACCEL_IIC_ADDR, MMA8451_WHO_AM_I, id, 1);
    #endif
	
    opencpu_i2c_write_read(ACCEL_IIC_ADDR, MMA8451_WHO_AM_I, id, 1);
    HalLog("id = %02x", id[0]);
    if(id[0] == ACCEL_MMA8451_ID)
    {
        //reset 
        
        data.reg = MMA8451_CTRL_REG2;
        data.value = 0x40;
        opencpu_i2c_write(ACCEL_IIC_ADDR, (uint8_t *)&data, sizeof(AccelRegSet_t));
        opencpu_delay_ms(5);

        do
        {
            opencpu_i2c_write_read(ACCEL_IIC_ADDR, MMA8451_CTRL_REG2, &value, 1);
            opencpu_delay_ms(5);
        }
        while (value & 0x40);
        
        data.reg = MMA8451_CTRL_REG1;
        data.value = 0x01;
        opencpu_i2c_write(ACCEL_IIC_ADDR, (uint8_t *)&data, sizeof(AccelRegSet_t));
        opencpu_delay_ms(1);
        
        data.reg = MMA8451_CTRL_REG2;
        data.value = 0x02;
        opencpu_i2c_write(ACCEL_IIC_ADDR, (uint8_t *)&data, sizeof(AccelRegSet_t));
        opencpu_delay_ms(1);

        data.reg = MMA8451_XYZ_DATA_CFG;
        data.value = 0x00;
        opencpu_i2c_write(ACCEL_IIC_ADDR, (uint8_t *)&data, sizeof(AccelRegSet_t));
        
        return 0;
    }
#endif
    return -1;
}

void AccelPoll(void)
{
    
}

