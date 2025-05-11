#ifndef BMP180_H
#define BMP180_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define BMP180_I2C_ADDR 0x77

#define BMP180_REG_CAL_AC1  0xAA
#define BMP180_REG_CAL_AC2  0xAC
#define BMP180_REG_CAL_AC3  0xAE
#define BMP180_REG_CAL_AC4  0xB0
#define BMP180_REG_CAL_AC5  0xB2
#define BMP180_REG_CAL_AC6  0xB4
#define BMP180_REG_CAL_B1   0xB6
#define BMP180_REG_CAL_B2   0xB8
#define BMP180_REG_CAL_MB   0xBA
#define BMP180_REG_CAL_MC   0xBC
#define BMP180_REG_CAL_MD   0xBE

#define BMP180_REG_CONTROL  0xF4
#define BMP180_REG_RESULT   0xF6

#define BMP180_CMD_TEMP     0x2E
#define BMP180_CMD_PRESSURE 0x34

#define OSS 0

#define BMP180_IOC_MAGIC 'b'
#define BMP180_IOCTL_GET_TEMP_C     _IOR(BMP180_IOC_MAGIC, 1, int)
#define BMP180_IOCTL_GET_PRESSURE   _IOR(BMP180_IOC_MAGIC, 2, int)
#define BMP180_IOCTL_GET_ALTITUDE   _IOR(BMP180_IOC_MAGIC, 3, int)

#endif