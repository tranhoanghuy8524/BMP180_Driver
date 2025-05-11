# Driver và Ứng Dụng Kiểm Thử cho Cảm Biến BMP180 trên Linux

## Giới thiệu

Driver này cung cấp giao diện cho người dùng để tương tác với cảm biến áp suất và nhiệt độ BMP180 thông qua giao thức I2C trên hệ thống Linux. Nó cho phép đọc nhiệt độ, áp suất và tính toán độ cao tương đối.

File `bmp180_driver.c` chứa mã nguồn của driver kernel.
File `user_test.c` là một ứng dụng người dùng đơn giản để kiểm tra các chức năng của driver.
File `bmp180.h` chứa các định nghĩa cần thiết cho cả driver và ứng dụng người dùng (ví dụ: các lệnh ioctl).

Driver này được phát triển bởi Hoàng Huy - Phương Huy - Phi Hùng - Nguyễn Khánh

## Yêu cầu

* Hệ thống Linux có hỗ trợ I2C.
* Kernel headers tương ứng với phiên bản kernel đang chạy.
* Công cụ biên dịch (ví dụ: `gcc`, `make`).
* Cảm biến BMP180 được kết nối đúng cách với bo mạch thông qua giao tiếp I2C.

## Cài đặt Driver

1.  **Sao chép các file driver:** Sao chép file `bmp180_driver.c` và `bmp180.h` vào thư mục nguồn driver của bạn (ví dụ: một thư mục `bmp180` trong thư mục bạn thường build kernel modules).

    ```bash
    mkdir bmp180_driver
    cd bmp180_driver
    # Giả sử bạn đã có bmp180_driver.c và bmp180.h trong thư mục này
    cp ../bmp180_driver.c .
    cp ../bmp180.h .
    ```

2.  **Tạo file `Makefile`:** Tạo một file `Makefile` trong cùng thư mục (`bmp180_driver`) với nội dung sau:

    ```makefile
    obj-m += bmp180_driver.o

    KDIR := /lib/modules/$(shell uname -r)/build
    PWD := $(shell pwd)

    default:
        $(MAKE) -C $(KDIR) M=$(PWD) modules

    clean:
        $(MAKE) -C $(KDIR) M=$(PWD) clean
    ```

3.  **Build driver:** Chạy lệnh `make` để biên dịch driver:

    ```bash
    make
    ```

    Lệnh này sẽ tạo ra file module `bmp180_driver.ko`.

4.  **Load driver:** Sử dụng lệnh `insmod` để load module vào kernel (cần quyền root):

    ```bash
    sudo insmod bmp180.ko
    ```

5.  **Kiểm tra device node:** Sau khi load, một device node `/dev/bmp180` sẽ được tạo. Kiểm tra sự tồn tại của nó:

    ```bash
    ls -l /dev/bmp180
    ```

## Sử dụng Driver

Driver này cung cấp các chức năng sau thông qua cơ chế `ioctl` trên device file `/dev/bmp180_driver`:

* `BMP180_IOCTL_GET_TEMP_C`: Lấy nhiệt độ theo độ Celsius (giá trị trả về đã được nhân với 10, ví dụ 255 tương ứng 25.5 °C).
* `BMP180_IOCTL_GET_PRESSURE`: Lấy áp suất theo đơn vị Pascal (Pa).
* `BMP180_IOCTL_GET_ALTITUDE`: Lấy độ cao tương đối so với áp suất chuẩn ở mực nước biển (101325 Pa), đơn vị mét.

## Chạy Ứng Dụng Kiểm Thử (`user_test.c`)

1.  **Sao chép file:** Sao chép file `user_test.c` và `bmp180.h` vào một thư mục (ví dụ, cùng thư mục với nơi bạn build driver).

2.  **Biên dịch ứng dụng:** Sử dụng `gcc` để biên dịch ứng dụng kiểm thử:

    ```bash
    gcc user_test.c -o user_test -lm
    ```

    Tham số `-lm` liên kết với thư viện toán học (cần cho hàm `pow` trong kernel, dù kết quả trả về user-space là số nguyên).

3.  **Chạy ứng dụng:** Đảm bảo driver `bmp180_driver.ko` đã được load, sau đó chạy ứng dụng:

    ```bash
    ./user_test
    ```

    Ứng dụng sẽ in ra các giá trị nhiệt độ, áp suất và độ cao đọc được từ cảm biến BMP180.

## Các Chức Năng Chi Tiết trong Driver

Driver `bmp180_driver.c` thực hiện các bước sau:

* **Đọc hệ số hiệu chuẩn:** Hàm `read_calibration_data` đọc các hệ số hiệu chuẩn từ các thanh ghi EEPROM của BMP180 và lưu trữ chúng trong cấu trúc `bmp180_calibration`.
* **Đọc giá trị chưa bù:**
    * `read_uncomp_temp`: Gửi lệnh để bắt đầu đo nhiệt độ và đọc giá trị nhiệt độ thô (UT).
    * `read_uncomp_pressure`: Gửi lệnh để bắt đầu đo áp suất (với độ phân giải OSS được định nghĩa) và đọc giá trị áp suất thô (UP).
* **Tính toán giá trị thực:**
    * `compute_temp`: Sử dụng UT và các hệ số hiệu chuẩn để tính toán nhiệt độ thực tế. Giá trị `b5` được lưu trữ để sử dụng trong tính toán áp suất. Nhiệt độ trả về được nhân với 10.
    * `compute_pressure`: Sử dụng UP, `b5` và các hệ số hiệu chuẩn để tính toán áp suất thực tế.
    * `compute_altitude`: Sử dụng áp suất đã tính toán để ước tính độ cao tương đối dựa trên công thức khí quyển tiêu chuẩn.
* **Giao diện `ioctl`:** Hàm `bmp180_ioctl` xử lý các lệnh `ioctl` từ không gian người dùng để trả về nhiệt độ, áp suất hoặc độ cao đã tính toán.
* **Tạo device file:** Driver đăng ký một character device (`/dev/bmp180_driver`) để người dùng có thể tương tác.
* **Quản lý module:** Các hàm `bmp180_probe` và `bmp180_remove` xử lý việc khởi tạo và giải phóng tài nguyên khi module được load và unload.

## Gỡ bỏ Driver

Để gỡ bỏ driver khỏi kernel:

```bash
sudo rmmod bmp180
```
