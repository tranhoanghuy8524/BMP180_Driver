Driver và Ứng Dụng Kiểm Thử cho Cảm Biến BMP180 trên Linux
BMP180 Sensor Driver and Test Application for Linux
🧾 Giới thiệu | Introduction
Driver này cung cấp giao diện cho người dùng để tương tác với cảm biến áp suất và nhiệt độ BMP180 thông qua giao thức I2C trên hệ thống Linux.
This driver provides a user interface to interact with the BMP180 pressure and temperature sensor via I2C on Linux systems.

Nó cho phép đọc nhiệt độ, áp suất và tính toán độ cao tương đối.
It supports reading temperature, pressure, and calculating relative altitude.

bmp180_driver.c: mã nguồn kernel driver

user_test.c: ứng dụng người dùng để kiểm thử

bmp180.h: định nghĩa chung (hằng số, lệnh ioctl)

bmp180_driver.ko: module kernel được biên dịch

👨‍💻 Tác giả / Authors: Hoàng Huy - Phương Huy - Phi Hùng - Nguyễn Khánh

🧰 Yêu cầu | Requirements
Linux system with I2C support

Kernel headers matching the running kernel

GCC or compatible compiler

BMP180 sensor connected correctly via I2C

🔧 Cài đặt Driver | Installing the Driver
1. Tạo thư mục và sao chép mã nguồn
Create a folder and copy source files

bash
Sao chép
Chỉnh sửa
mkdir bmp180_driver
cd bmp180_driver
cp ../bmp180_driver.c .
cp ../bmp180.h .
2. Tạo Makefile
Create a Makefile in the same directory:

Makefile
Sao chép
Chỉnh sửa
obj-m += bmp180_driver.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
3. Build driver
bash
Sao chép
Chỉnh sửa
make
Kết quả sẽ tạo file bmp180_driver.ko.
This will generate bmp180_driver.ko.

4. Load module
bash
Sao chép
Chỉnh sửa
sudo insmod bmp180_driver.ko
5. Kiểm tra thiết bị | Check device node
bash
Sao chép
Chỉnh sửa
ls -l /dev/bmp180
📡 Sử dụng Driver | Using the Driver
Driver hỗ trợ các lệnh thông qua ioctl:
The driver provides the following ioctl commands:

Lệnh ioctl	Mô tả (VN)	Description (EN)
BMP180_IOCTL_GET_TEMP_C	Lấy nhiệt độ (x10)	Get temperature (×10)
BMP180_IOCTL_GET_PRESSURE	Lấy áp suất (Pa)	Get pressure (Pa)
BMP180_IOCTL_GET_ALTITUDE	Tính độ cao tương đối (m)	Compute relative altitude (m)

🧪 Chạy Ứng Dụng Kiểm Thử | Running the Test Application
1. Chuẩn bị mã nguồn
Prepare user_test.c and bmp180.h in a folder.

2. Biên dịch | Compile
bash
Sao chép
Chỉnh sửa
gcc user_test.c -o user_test -lm
3. Chạy chương trình | Run the test
bash
Sao chép
Chỉnh sửa
./user_test
Kết quả in ra nhiệt độ, áp suất và độ cao.
It will display temperature, pressure, and altitude readings.

🔬 Giải thích Driver | Driver Details
Hiệu chuẩn / Calibration:
read_calibration_data() đọc hệ số EEPROM của cảm biến.

Đọc dữ liệu thô / Read uncompensated values:

read_uncomp_temp()

read_uncomp_pressure()

Tính toán / Compute results:

compute_temp(): Trả về nhiệt độ thực (×10)

compute_pressure(): Trả về áp suất (Pa)

compute_altitude(): Trả về độ cao (m)

Giao tiếp người dùng / User-space interface:
Thông qua ioctl và thiết bị /dev/bmp180.

❌ Gỡ bỏ Driver | Unload the Driver
bash
Sao chép
Chỉnh sửa
sudo rmmod bmp180_driver
📁 Cấu trúc thư mục gợi ý | Suggested Directory Structure
Sao chép
Chỉnh sửa
bmp180_project/
├── bmp180_driver/
│   ├── bmp180_driver.c
│   ├── bmp180.h
│   ├── Makefile
├── user_test/
│   ├── user_test.c
│   ├── bmp180.h
🌐 License
This project is licensed under the GPL.
Dự án được phát hành theo giấy phép GPL.
