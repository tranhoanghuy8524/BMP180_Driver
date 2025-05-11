# Driver và Ứng Dụng Kiểm Thử cho Cảm Biến BMP180 trên Linux

## 1. Giới thiệu

Driver này cung cấp giao diện cho người dùng để tương tác với cảm biến áp suất và nhiệt độ BMP180 thông qua giao thức I2C trên hệ thống Linux. Nó cho phép đọc các thông số quan trọng như nhiệt độ, áp suất khí quyển và ước tính độ cao tương đối.

Dự án này bao gồm:

* `bmp180_driver.c`: Mã nguồn của driver kernel cho cảm biến BMP180.
* `bmp180.h`: File header chứa các định nghĩa, cấu trúc và các lệnh `ioctl` được sử dụng bởi cả driver và ứng dụng người dùng.
* `user_test.c`: Một ứng dụng người dùng đơn giản để minh họa cách sử dụng driver và đọc dữ liệu từ cảm biến.
* `README.md`: Tài liệu hướng dẫn sử dụng này.

Driver này được phát triển bởi nhóm tác giả: HoangHuy, PhuongHuy, PhiHung, NguyenKhanh.

## 2. Yêu cầu

Để biên dịch và sử dụng driver này, hệ thống của bạn cần đáp ứng các yêu cầu sau:

* Hệ thống Linux có hỗ trợ giao tiếp I2C (đã cấu hình và có các module kernel liên quan).
* Kernel headers tương ứng với phiên bản kernel đang chạy trên hệ thống.
* Bộ công cụ phát triển C/C++ (ví dụ: `gcc`, `make`).
* Cảm biến BMP180 được kết nối chính xác với bo mạch chủ thông qua giao tiếp I2C. Bạn cần xác định địa chỉ I2C của cảm biến (thường là `0x77` hoặc `0x76`). Driver này mặc định hoạt động với địa chỉ được định nghĩa trong code (`BMP180_I2C_ADDR`).

## 3. Cài đặt Driver Kernel

Các bước sau đây hướng dẫn cách biên dịch và cài đặt driver kernel:

1.  **Sao chép các file driver:** Tạo một thư mục cho driver và sao chép các file `bmp180_driver.c` và `bmp180.h` vào thư mục đó.

    ```bash
    mkdir bmp180_driver
    cd bmp180_driver
    # Giả sử bạn đã có bmp180_driver.c và bmp180.h
    cp ../bmp180_driver.c .
    cp ../bmp180.h .
    ```

2.  **Tạo file `Makefile`:** Tạo một file `Makefile` trong cùng thư mục với nội dung sau:

    ```makefile
    obj-m += bmp180_driver.o

    KDIR := /lib/modules/$(shell uname -r)/build
    PWD := $(shell pwd)

    default:
        $(MAKE) -C $(KDIR) M=$(PWD) modules

    clean:
        $(MAKE) -C $(KDIR) M=$(PWD) clean
    ```

3.  **Biên dịch driver:** Sử dụng lệnh `make` để biên dịch driver kernel:

    ```bash
    make
    ```

    Lệnh này sẽ tạo ra file module kernel `bmp180_driver.ko`.

4.  **Load driver vào kernel:** Sử dụng lệnh `insmod` với quyền `sudo` để load module vào kernel:

    ```bash
    sudo insmod bmp180_driver.ko
    ```

5.  **Kiểm tra device node:** Sau khi driver được load thành công, một device node có tên `bmp180` sẽ được tạo trong thư mục `/dev/`. Bạn có thể kiểm tra nó bằng lệnh:

    ```bash
    ls -l /dev/bmp180
    ```

## 4. Sử dụng Driver

Driver này cung cấp các chức năng đọc dữ liệu cảm biến thông qua cơ chế `ioctl` trên device file `/dev/bmp180`. Các lệnh `ioctl` được định nghĩa trong file `bmp180.h`:

* `BMP180_IOCTL_GET_TEMP_C`: Lấy nhiệt độ theo độ Celsius. Giá trị trả về là một số nguyên, đã được nhân với 10 để giữ độ chính xác một chữ số thập phân (ví dụ: 255 tương ứng với 25.5 °C).
* `BMP180_IOCTL_GET_PRESSURE`: Lấy áp suất khí quyển theo đơn vị Pascal (Pa).
* `BMP180_IOCTL_GET_ALTITUDE`: Lấy độ cao tương đối so với áp suất mực nước biển tiêu chuẩn (101325 Pa), đơn vị mét.

## 5. Chạy Ứng Dụng Kiểm Thử (`user_test.c`)

Ứng dụng `user_test.c` là một ví dụ đơn giản về cách tương tác với driver BMP180.

1.  **Sao chép file:** Đảm bảo bạn có các file `user_test.c` và `bmp180.h` trong cùng một thư mục.

2.  **Biên dịch ứng dụng:** Sử dụng `gcc` để biên dịch ứng dụng:

    ```bash
    gcc user_test.c -o user_test -lm
    ```

    Tham số `-lm` được thêm vào để liên kết với thư viện toán học (cần thiết cho hàm `pow` được sử dụng trong kernel để tính toán độ cao).

3.  **Chạy ứng dụng:** Đảm bảo driver `bmp180_driver.ko` đã được load vào kernel. Sau đó, chạy ứng dụng kiểm thử với quyền người dùng thông thường:

    ```bash
    ./user_test
    ```

    Ứng dụng sẽ mở device file `/dev/bmp180` và sử dụng các lệnh `ioctl` để đọc nhiệt độ, áp suất và độ cao, sau đó in các giá trị này ra màn hình.

## 6. Các Chức Năng Chi Tiết trong Driver (`bmp180_driver.c`)

Driver kernel `bmp180_driver.c` thực hiện các tác vụ sau:

* **Khởi tạo và thăm dò thiết bị:** Hàm `bmp180_probe` được gọi khi một thiết bị BMP180 được phát hiện trên bus I2C. Nó cấp phát bộ nhớ cho cấu trúc dữ liệu của driver, lưu trữ con trỏ `i2c_client`, đọc các hệ số hiệu chuẩn từ EEPROM của cảm biến bằng hàm `read_calibration_data`, đăng ký character device và tạo device file `/dev/bmp180`.
* **Đọc dữ liệu hiệu chuẩn:** Hàm `read_calibration_data` đọc 11 hệ số hiệu chuẩn (AC1-AC6, B1, B2, MB, MC, MD) từ các thanh ghi tương ứng của BMP180. Các hệ số này rất quan trọng để chuyển đổi các giá trị thô thành nhiệt độ và áp suất chính xác.
* **Đọc giá trị chưa bù:**
    * `read_uncomp_temp`: Gửi lệnh đến BMP180 để bắt đầu quá trình đo nhiệt độ và sau đó đọc giá trị nhiệt độ thô (UT).
    * `read_uncomp_pressure`: Tương tự, gửi lệnh để đo áp suất với độ phân giải lấy mẫu (OSS) được cấu hình và đọc giá trị áp suất thô (UP).
* **Tính toán giá trị thực:**
    * `compute_temp`: Sử dụng giá trị UT và các hệ số hiệu chuẩn để tính toán nhiệt độ thực tế theo thuật toán được mô tả trong datasheet của BMP180. Giá trị trung gian `b5` được lưu trữ để sử dụng trong tính toán áp suất. Nhiệt độ trả về được nhân với 10.
    * `compute_pressure`: Sử dụng giá trị UP, `b5` và các hệ số hiệu chuẩn để tính toán áp suất khí quyển thực tế.
    * `compute_altitude`: Ước tính độ cao tương đối dựa trên áp suất hiện tại và áp suất tham chiếu ở mực nước biển (mặc định là 101325 Pa). Công thức được sử dụng dựa trên mô hình khí quyển tiêu chuẩn.
* **Xử lý lệnh `ioctl`:** Hàm `bmp180_ioctl` là điểm vào cho các lệnh `ioctl` từ không gian người dùng. Dựa trên lệnh được nhận (`cmd`), nó sẽ gọi các hàm tương ứng để đọc nhiệt độ, áp suất hoặc tính toán độ cao và trả về kết quả cho ứng dụng người dùng.
* **Gỡ bỏ driver:** Hàm `bmp180_remove` được gọi khi module driver được gỡ bỏ. Nó thực hiện các thao tác dọn dẹp như hủy device, hủy class và giải phóng bộ nhớ đã cấp phát.

## 7. Ví dụ ứng dụng

### 7.1. Trạm thời tiết đơn giản

#### 7.1.1. Sơ đồ kết nối (ví dụ với Raspberry Pi)

* BMP180 VCC $\rightarrow$ 3.3V Raspberry Pi
* BMP180 GND $\rightarrow$ GND Raspberry Pi
* BMP180 SDA $\rightarrow$ SDA (GPIO 2) Raspberry Pi
* BMP180 SCL $\rightarrow$ SCL (GPIO 3) Raspberry Pi

#### 7.1.2. Code ví dụ

Sử dụng đoạn chương trình ở mục 5 để thu thập dữ liệu và hiển thị ra terminal hoặc ghi vào file log.

### 7.2. Đo độ cao cho dự án bay

#### 7.2.1. Sơ đồ kết nối

Kết nối tương tự như trạm thời tiết.

#### 7.2.2. Code ví dụ

Ứng dụng `user_test.c` đã bao gồm chức năng đọc độ cao. Bạn có thể sử dụng giá trị trả về để điều khiển các hệ thống khác trong dự án bay của mình.

Tính độ cao từ áp suất theo công thức:

float altitude = 44330.0 * (1.0 - pow((float)pressure / 101325.0, 0.1903));

(Công thức này đã được tích hợp vào driver kernel).


## 8. Gỡ bỏ Driver Kernel

Để gỡ bỏ driver kernel khỏi hệ thống, sử dụng lệnh sau với quyền `sudo`:

```bash
sudo rmmod bmp180_driver
"
