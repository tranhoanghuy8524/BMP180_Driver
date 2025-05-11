/*
Thanh vien nhom:
Tran Hoang Huy - 22146316
Ngo Phuong Huy - 22146313
Nguyen Phi Hung - 22146319
Nguyen Khanh -22146331
*/

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/math64.h>
#include "bmp180.h"

#define DEVICE_NAME "bmp180"
#define CLASS_NAME "bmp180_class"

// Cau truc luu tru he so hieu chuan doc tu eeprom
struct bmp180_calibration {
    s16 ac1, ac2, ac3;
    u16 ac4, ac5, ac6;
    s16 b1, b2, mb, mc, md;
};

// Cau truc du lieu chinh
struct bmp180_data {
    struct i2c_client *client;
    struct bmp180_calibration calib;
    dev_t devt;
    struct class *class;
    struct cdev cdev;
    s32 b5; // tinh nhiet do ap suat
};

static struct bmp180_data *bmp_dev;
// 16bit co dau
static s16 read_s16(struct i2c_client *client, u8 reg) {
    u8 buf[2];
    i2c_smbus_read_i2c_block_data(client, reg, 2, buf);
    return (buf[0] << 8) | buf[1];
}

//16bit khong dau
static u16 read_u16(struct i2c_client *client, u8 reg) {
    u8 buf[2];
    i2c_smbus_read_i2c_block_data(client, reg, 2, buf);
    return (buf[0] << 8) | buf[1];
}

//Doc he so trang 13, muc3.4
static void read_calibration_data(struct bmp180_data *dev) {
    struct i2c_client *client = dev->client;
    dev->calib.ac1 = read_s16(client, BMP180_REG_CAL_AC1);
    dev->calib.ac2 = read_s16(client, BMP180_REG_CAL_AC2);
    dev->calib.ac3 = read_s16(client, BMP180_REG_CAL_AC3);
    dev->calib.ac4 = read_u16(client, BMP180_REG_CAL_AC4);
    dev->calib.ac5 = read_u16(client, BMP180_REG_CAL_AC5);
    dev->calib.ac6 = read_u16(client, BMP180_REG_CAL_AC6);
    dev->calib.b1 = read_s16(client, BMP180_REG_CAL_B1);
    dev->calib.b2 = read_s16(client, BMP180_REG_CAL_B2);
    dev->calib.mb = read_s16(client, BMP180_REG_CAL_MB);
    dev->calib.mc = read_s16(client, BMP180_REG_CAL_MC);
    dev->calib.md = read_s16(client, BMP180_REG_CAL_MD);
}

//Nhiet do chua bu trang 14 muc 3.5
static s32 read_uncomp_temp(struct i2c_client *client) {
    i2c_smbus_write_byte_data(client, BMP180_REG_CONTROL, BMP180_CMD_TEMP);
    msleep(5);
    return read_u16(client, BMP180_REG_RESULT);
}

//Ap suat chua bu trang 14 muc 3.5
static s32 read_uncomp_pressure(struct i2c_client *client) {
    i2c_smbus_write_byte_data(client, BMP180_REG_CONTROL, BMP180_CMD_PRESSURE + (OSS << 6));
    msleep(8);
    u8 msb = i2c_smbus_read_byte_data(client, BMP180_REG_RESULT);
    u8 lsb = i2c_smbus_read_byte_data(client, BMP180_REG_RESULT + 1);
    u8 xlsb = i2c_smbus_read_byte_data(client, BMP180_REG_RESULT + 2);
    return ((msb << 16) | (lsb << 8) | xlsb) >> (8 - OSS);
}

//Tinh nhiet do thuc trang 15 muc 3.5
static s32 compute_temp(struct bmp180_data *dev, s32 ut) {
    s32 x1 = ((ut - dev->calib.ac6) * dev->calib.ac5) >> 15;
    s32 x2 = (dev->calib.mc << 11) / (x1 + dev->calib.md);
    dev->b5 = x1 + x2;
    return (dev->b5 + 8) >> 4;
}

//Tinh ap suat thuc te trang15 muc3.5
static s32 compute_pressure(struct bmp180_data *dev, s32 up) {
    s32 b6 = dev->b5 - 4000;
    s32 x1 = (dev->calib.b2 * ((b6 * b6) >> 12)) >> 11;
    s32 x2 = (dev->calib.ac2 * b6) >> 11;
    s32 x3 = x1 + x2;
    s32 b3 = (((dev->calib.ac1 * 4 + x3) << OSS) + 2) >> 2;
    x1 = (dev->calib.ac3 * b6) >> 13;
    x2 = (dev->calib.b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    u32 b4 = (dev->calib.ac4 * (u32)(x3 + 32768)) >> 15;
    u32 b7 = ((u32)up - b3) * (50000 >> OSS);
    s32 p = (b7 < 0x80000000) ? (b7 << 1) / b4 : (b7 / b4) << 1;
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p += (x1 + x2 + 3791) >> 4;
    return p;
}

//Tinh do cao tuong doi
static s32 compute_altitude(s32 pressure) {
    double ratio = (double)pressure / 101325.0;
    return (s32)(44330.0 * (1.0 - pow(ratio, 0.1903)));
}

// in out
static long bmp180_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    int result;
    s32 ut, up;

    switch (cmd) {
        case BMP180_IOCTL_GET_TEMP_C:
            ut = read_uncomp_temp(bmp_dev->client);
            result = compute_temp(bmp_dev, ut);
            break;
        case BMP180_IOCTL_GET_PRESSURE:
            ut = read_uncomp_temp(bmp_dev->client);
            compute_temp(bmp_dev, ut);
            up = read_uncomp_pressure(bmp_dev->client);
            result = compute_pressure(bmp_dev, up);
            break;
        case BMP180_IOCTL_GET_ALTITUDE:
            ut = read_uncomp_temp(bmp_dev->client);
            compute_temp(bmp_dev, ut);
            up = read_uncomp_pressure(bmp_dev->client);
            result = compute_altitude(compute_pressure(bmp_dev, up));
            break;
        default:
            return -EINVAL;
    }

    return copy_to_user((int __user *)arg, &result, sizeof(int)) ? -EFAULT : 0;
}

static struct file_operations bmp180_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = bmp180_ioctl,
};

//
static int bmp180_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    int ret;
    bmp_dev = kzalloc(sizeof(*bmp_dev), GFP_KERNEL);
    bmp_dev->client = client;
    read_calibration_data(bmp_dev);

    ret = alloc_chrdev_region(&bmp_dev->devt, 0, 1, DEVICE_NAME);
    cdev_init(&bmp_dev->cdev, &bmp180_fops);
    cdev_add(&bmp_dev->cdev, bmp_dev->devt, 1);
    bmp_dev->class = class_create(THIS_MODULE, CLASS_NAME);
    device_create(bmp_dev->class, NULL, bmp_dev->devt, NULL, DEVICE_NAME);

    pr_info("BMP180 driver probed\n");
    return 0;
}

//
static void bmp180_remove(struct i2c_client *client) {
    device_destroy(bmp_dev->class, bmp_dev->devt);
    class_destroy(bmp_dev->class);
    cdev_del(&bmp_dev->cdev);
    unregister_chrdev_region(bmp_dev->devt, 1);
    kfree(bmp_dev);
    pr_info("BMP180 driver removed\n");
}

static const struct i2c_device_id bmp180_ids[] = {
    {"bmp180", 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, bmp180_ids);

static struct i2c_driver bmp180_driver = {
    .driver = {
        .name = DEVICE_NAME,
    },
    .probe = bmp180_probe,
    .remove = bmp180_remove,
    .id_table = bmp180_ids,
};

module_i2c_driver(bmp180_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HoangHuy_PhuongHuy_PhiHung_NguyenKhanh");
MODULE_DESCRIPTION("BMP180 with temperature, pressure, and altitude");
