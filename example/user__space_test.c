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
    close(fd);
    return 0;
}
