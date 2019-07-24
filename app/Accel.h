#ifndef ACCEL_H
#define ACCEL_H

#define MMA8451_STATUS				0x00
#define MMA8451_OUT_X_MSB     		0x01
#define MMA8451_OUT_X_LSB     		0x02
#define MMA8451_OUT_Y_MSB     		0x03
#define MMA8451_OUT_Y_LSB     		0x04
#define MMA8451_OUT_Z_MSB     		0x05
#define MMA8451_OUT_Z_LSB     		0x06
//Reserved								0x07-0x08
#define MMA8451_F_SETUP				0x09
#define MMA8451_TRIG_CFG			0x0A
#define MMA8451_SYSMOD				0x0B
#define MMA8451_INT_SOURCE			0x0C
#define MMA8451_WHO_AM_I      		0x0D
#define MMA8451_XYZ_DATA_CFG		0x0E
#define MMA8451_HP_FILTER_CUTOFF	0x0F
#define MMA8451_PL_STATUS			0x10
#define MMA8451_PL_CFG				0x11
#define MMA8451_PL_COUNT			0x12
#define MMA8451_PL_BF_ZCOMP			0x13
#define MMA8451_P_L_THS_REG			0x14
#define MMA8451_FF_MT_CFG			0x15
#define MMA8451_FF_MTSRC			0x16
#define MMA8451_FF_MT_THS			0x17
#define MMA8451_FF_MT_COUNT			0x18
//Reserved								0x19-0x1C
#define MMA8451_TRANSIENT_CFG		0x1D
#define MMA8451_TRANSIENT_SCR		0x1E
#define MMA8451_CTRL_REG1     		0x2A
#define MMA8451_CTRL_REG2			0x2B
#define MMA8451_CTRL_REG3			0x2C
#define MMA8451_CTRL_REG4			0x2D
#define MMA8451_CTRL_REG5			0x2E
#define MMA8451_OFF_X				0x2F
#define MMA8451_OFF_Y				0x30
#define MMA8451_OFF_Z				0x31
//Reserved								0x40-0x7F

signed short *AccelReadAxis(void);
int AccelInit(void);
void AccelPoll(void);
#endif
