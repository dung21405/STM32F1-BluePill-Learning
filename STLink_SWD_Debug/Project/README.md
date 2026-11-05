# STM32F103C8T6 - ST-Link SWD Debug

Project STM32F103C8T6 (Blue Pill), sử dụng:

- ARM GCC (`arm-none-eabi-gcc`, `gdb-multiarch`)
- STM32F10x Standard Peripheral Library
- ST-Link qua giao tiếp SWD (Serial Wire Debugger)
- OpenOCD để nạp firmware và kết nối với GDB server
- `st-info`/`st-util` trong bộ công cụ ST-Link

## Flow hoạt động qua các layer
User/GDB → OpenOCD → USB/ST-Link → SWDIO/SWCLK/NRST → Cortex-M3 target → voltage/
reset/halt observation


### 1. Kết nối phần cứng

Kết nối ST-Link với Blue Pill như sau:

| ST-Link | STM32F103C8T6 |
|---|---|
| SWDIO | PA13 / SWDIO |
| SWCLK | PA14 / SWCLK |
| GND | GND |
| 3.3V | 3.3V (nếu cấp nguồn từ ST-Link) |
| NRST (khuyến nghị) | NRST |


### 2. Build project

Mở terminal tại thư mục chứa file `Makefile`:

```bash
cd /home/dung/Documents/Arm/BluePill_learning/STLink_SWD_Debug/Project
```

Build mặc định:
```bash
make clean && make
```

### 3. Output project
Các file đầu ra nằm trong thư mục `build/`:
- `build/firmware.elf`: chứa mã máy, symbol, tên hàm, địa chỉ, debug info
- `build/firmware.bin`: file nhị phân
- `build/firmware.hex`: chứa mã máy dạng hex và địa chỉ
- `build/firmware.map`: thông tin phân bố section và symbol

Một số lệnh hữu ích:

```bash
make size                         # Xem dung lượng code/data
make list                         # Tạo build/firmware.list
make build/firmware.hex           # Tạo file Intel HEX
make clean                        # Xóa ELF/BIN/HEX/MAP/LIST và object
make clean-all                    # Xóa toàn bộ thư mục build/
```

Build cho Renode (nếu cần):
```bash
make clean
make TARGET_ENV=renode
```

### 4. Nạp firmware bằng OpenOCD
Nạp trực tiếp bằng target có sẵn trong Makefile:
```bash
make flash
```
Lệnh kết nối ST-Link ở SWD, halt MCU, nạp `build/firmware.elf`, verify, reset rồi thoát.

Dùng file `.elf` để OpenOCD tự lấy địa chỉ section:

```bash
openocd -f openocd.cfg \
	-c "init; halt; program build/firmware.elf verify; reset run; shutdown"
```

`openocd.cfg` của project chọn `hla_swd`, đặt `CPUTAPID` của STM32F1 và tốc độ adapter 1 MHz. Nếu kết nối không ổn định, thử giảm tốc độ xuống 100 hoặc 500 kHz.

### 5. Chạy OpenOCD ở chế độ GDB server

Terminal 1, khởi động OpenOCD và để terminal chạy với file config thông số ở `openoc.cfg`:

```bash
openocd -f openocd.cfg
```

Terminal 1, khởi động OpenOCD với transport mặc định HLA_SWD
```bash
openocd -c "set CPUTAPID 0x1ba01477" \
  -f interface/stlink.cfg \
  -f target/stm32f1x.cfg
```
Terminal 1, khởi động OpenOCD với transport ở chế độ Direct-DAP
```bash
openocd -c "set CPUTAPID 0x2ba01477" \
        -f interface/stlink-dap.cfg \
        -f target/stm32f1x.cfg
```

Mặc định OpenOCD cung cấp:
- GDB server tại `localhost:3333`
- Telnet command interface tại `localhost:4444`

Terminal 2, kết nối bằng ARM GDB:
```bash
gdb-multiarch build/firmware.elf		#Đúng đường dẫn chứa file .elf
```

### 6. Giao tiếp Telnet với OpenOCD

Khi OpenOCD đang chạy, mở terminal khác:

```bash
telnet localhost 4444
```
Một số lệnh OpenOCD thường dùng:

```bash
version
adapter speed 1000
reset halt
halt
resume
reset run
reg
mdw 0x08000000 8
mdw 0x20000000 8
mww 0x20000000 0x12345678
flash info 0
exit
```

Thoát Telnet bằng `exit` hoặc `Ctrl-]`, sau đó nhập `quit`.

### 7. Kiểm tra ST-Link bằng các lệnh probe

### `st-info`

Kiểm tra ST-Link và thông tin chip:
```bash
st-info --probe
st-info --descr
st-info --chipid
st-info --serial
st-info --flash
st-info --sram
```

Nếu `st-info --probe` không tìm thấy thiết bị, kiểm tra cáp USB, nguồn, dây `SWDIO/SWCLK/GND` và quyền truy cập USB.
### `st-util`

`st-util` là một GDB server khác của bộ công cụ ST-Link, thường lắng nghe tại cổng `4242`:
```bash
st-util
```

Kết nối bằng GDB ở terminal khác:
```bash
arm-none-eabi-gdb build/firmware.elf
```

```bash
target extended-remote localhost:4242
monitor reset halt
break main
continue
```

Chỉ chạy một GDB server tại một thời điểm. Hãy dừng OpenOCD trước khi chạy `st-util`, và ngược lại.

### `probe-rs` (tùy chọn)

Nếu cài thêm `probe-rs`, có thể kiểm tra probe và chip bằng:

```bash
probe-rs list
probe-rs info
probe-rs erase --chip STM32F103C8
probe-rs download --chip STM32F103C8 build/firmware.elf --format elf
probe-rs run --chip STM32F103C8 build/firmware.elf
```

Tên chip có thể khác theo phiên bản probe-rs. Dùng `probe-rs chip list | grep STM32F103` để tìm tên chính xác. Không cần cài `probe-rs` nếu sử dụng OpenOCD hoặc `st-util`.

### 8. Xử lý lỗi thường gặp

### `arm-none-eabi-gcc: command not found`

Thêm thư mục chứa ARM GNU Toolchain vào `PATH`, ví dụ:

```bash
export PATH="/opt/st/stm32cubeclt_1.19.0/GNU-tools-for-STM32/bin:$PATH"
```

Kiểm tra:

```bash
arm-none-eabi-gcc --version
arm-none-eabi-gdb --version
```

### OpenOCD không nhận ST-Link

```bash
lsusb | grep -i st
st-info --probe
```

Giảm tốc độ SWD xuống 100kHz:
```bash
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
	-c "transport select hla_swd" -c "adapter speed 100"
```

### GDB báo không kết nối được cổng 3333
Kiểm tra OpenOCD có đang chạy và cổng có lắng nghe:
```bash
ss -ltnp | grep -E '3333|4444'
```
Nếu đang dùng `st-util`, GDB phải kết nối tới `localhost:4242` thay vì `localhost:3333`.

### Không dừng được tại breakpoint
Đảm bảo đang dùng `build/firmware.elf`, không phải `.bin`, vì ELF chứa symbol debug. Có thể reset và halt lại:
```gdb
monitor reset halt
thbreak main
continue
```

# GDB-Multiarch

## Debug GDB chỉ với file .elf ở host
## 1. Khởi động GDB offline và Tra cứu hàm, biến và địa chỉ (Symbol Table)
```bash
gdb-multiarch /built/firmware.elf
info functions 			# Liệt kê tất cả các hàm có trong tệp ELF.
info address main 		# In địa chỉ Flash nơi hàm main được đặt
print &known_var 		# In địa chỉ bộ nhớ RAM mà Linker đã cấp phát cho biến known_var
ptype known_var 		# In kiểu dữ liệu (struct/enum/typedef) của biến known_var
```

## 2. Xem mã nguồn C và bóc tách mã máy (Disassembly)
```bash
list main 							# Hiển thị các dòng code C trong hàm main (yêu cầu khi biên dịch tệp ELF có bật flag -g).
disassemble main 					# Bóc tách toàn bộ hàm main thành các lệnh Assembly tương ứng.
disassemble 0x08000000, 0x08000040 	# Bóc tách mã Assembly trong phạm vi địa chỉ Flash chỉ định
info line main 						# Hiển thị khoảng địa chỉ bộ nhớ tương ứng với hàm main.
info line *0x08000010 				# Tra ngược lại xem địa chỉ Hex 0x08000010 thuộc dòng mã C nào trong tệp nguồn
```

## 3. Kiểm tra cấu trúc phân đoạn bộ nhớ (Sections)
```bash
info target					 		# Hiển thị danh sách các phân đoạn (.text, .rodata, .data, .bss), địa chỉ nạp (VMA/LMA) và kích thước của từng đoạn.
maintenance info sections 			# Xem thông tin chi tiết về các cờ (flags) và vị trí offset của từng section trong tệp ELF.
```

## GDB kết nối với STLink thông qua OpenOCD
### 1. Kết nối, nạp firmware và đồng bộ trạng thái phần cứng

```bash 
target extended-remote :3333 	 # Kết nối tới OpenOCD linh hoạt và giữ phiên connect kể cả khi target reset.
target remote localhost:3333	 # Kết nối với OpenOCD với cổng TCP với port :3333 
monitor reset halt				 # Reset và đưa CPU vào trạng thái dừng
info registers pc msp lr xPSR 	 # Kiểm tra các thanh ghi 
x/2wx 0x08000000				 # Đọc 2 work (32bit hex) đầu tiên của flash (initial MSP và Reset_Handler)	
x/1wx 0xE000ED00				 # Đọc thanh ghi CPUID để xác nhận lõi Cortex-M3 (0x411fc231)
maintenance flush register-cache # Xóa bộ nhớ cache và đọc lại giá trị thực từ lõi Cortex-M3
load  							 # Nạp trực tiếp file .elf hiện tại xuống Flash hoặc SRAM thông qua GDB.
compare-sections				 # Đối chiếu .elf ở host và flash ở target
monitor reset halt				 # Giử lệnh xuống OpenOCD để đưa CPU vào trạng thái reset và dừng

```

### 2. Quản lý Breakpoint và Watchpoint nâng cao (Tối ưu tài nguyên FPB/DWT)
```bash
break main			# Đặt breakpoint tại main
info b				# Kiểm tra các breakpoint kiện có
delete <num>		# Xóa breakpoint theo num
```

### 3. Kiểm tra vùng nhớ thô, ngoại vi (MMIO) và cấu trúc dữ liệu
```bash
x/8wx 0x08000000				# Đọc 8 word (32-bit hex) tại đầu vùng nhớ Flash để kiểm tra Vector table
x/1wx 0x40021018				# Đọc trực tiếp thanh ghi ngoại vi theo địa chỉ vật lý	(RCC_APB2ENR)
x/4i $pc						# Dịch ngược (disassemble) 4 lệnh hợp ngữ kế tiếp tính từ vị trí PC hiện tại

```
### 4. Điều tra lỗi Crash, Ngắt (Exception) và Stack Frame (HardFault)
```bash
ptype <struct or value>			# In ra định nghĩa chi tiết các trường của một struct phức tạp.

```

### 5. Thay đổi luồng chạy và biến ngay trong lúc chạy (Live Patching)
```bash
set var value = 0x1234				# Thay đổi giá trị biến trong SRAM ngay lập tức
set {uint32_t}0x4001100C = 0x2000	# Ghi trực tiếp giá trị 32-bit vào một địa chỉ thanh ghi vật lý
set $pc = 0x20000000				# Thay đổi PC để ép CPU nhảy tới thực thi một vùng code khác (SRAM)
```

### 6. Đóng băng ngoại vi khi dừng CPU và tự động hóa bằng
```bash
monitor mww 0xE0042004 0x00000800		# Đóng băng Timer khi Debug
```

### 7. Kết thúc phiên làm việc và in ra file log để kiểm chứng
Các lệnh kết thúc phiên:
```bash
monitor reset run				#reset and run
detach							#Ngắt kết nối giữa GDB và mục tiêu (target/OpenOCD) mà không tắt hẳn chương trình GDB. Trạng thái của MCU sau khi detach phụ thuộc vào việc bạn đã reset nó chưa.
set logging file <name>.log		# Chỉ định tên file dùng để lưu log
set logging enabled on        # Enable khi muon ghi lai log
show logging					# Kiểm tra xem tính năng logging hiện đang bật hay tắt và tên file đang ghi là gì.
monitor reset halt				# Reset and halt program
monitor halt
monitor soft_reset_halt			# Reset phần mềm nhẹ nhàng hơn và đưa chip về trạng thái dừng
interrupt						# Tương đương với việc nhấn nút Pause trong các IDE giao diện; dùng để dừng chương trình đang chạy vòng lặp vô hạn trên chip để bạn kiểm tra giá trị biến hoặc thanh ghi.
quit

```


