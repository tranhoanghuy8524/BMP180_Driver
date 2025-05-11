/*
Thanh vien nhom:
Tran Hoang Huy  - 22146316
Ngo Phuong Huy  - 22146313
Nguyen Phi Hung - 22146319
Nguyen Khanh    - 22146331
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>
#include "bmp180.h"

int main() {
    int fd;
    int result;

    fd = open("/dev/bmp180", O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return 1;
    }
    // In nhiet do
    if (ioctl(fd, BMP180_IOCTL_GET_TEMP_C, &result) == 0) {
        printf("Temperature: %f C\n", (float)result / 10.0);
    } else {
        perror("ioctl (get temp) failed");
    }
    // In ap suat
    if (ioctl(fd, BMP180_IOCTL_GET_PRESSURE, &result) == 0) {
        printf("Pressure: %d Pa\n", result);
    } else {
        perror("ioctl (get pressure) failed");
    }
    // In do cao
    if (ioctl(fd, BMP180_IOCTL_GET_ALTITUDE, &result) == 0) {
        printf("Altitude: %d meters\n", result);
    } else {
        perror("ioctl (get altitude) failed");
    }
    close(fd);
    return 0;
}
