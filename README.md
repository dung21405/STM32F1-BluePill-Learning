# STM32F1-BluePill-Learning
Learning STM32F1 architecture, peripherals, and embedded C programming with the STM32F103C8T6 Blue Pill board.

### Buoi1: STLink-SWD-Debug (`./STLink_SWD_Debug`)
* **Nội dung:** Tìm hiểu kiến trúc gỡ lỗi nhiều lớp (`GDB -> OpenOCD -> ST-Link -> SWD -> CoreSight DAP -> STM32F103`) và phân biệt quy trình `Build -> Flash -> Verify -> Boot -> Debug`.
* **Thực hành:** Cấu hình OpenOCD, nạp/xác minh `firmware.elf`, điều khiển CPU qua GDB (`halt`, `reset`, `step`, `breakpoint`), đọc thanh ghi (`PC`, `SP`, `LR`), kiểm tra SRAM và chẩn đoán lỗi theo lớp.
* **Chi tiết & Hướng dẫn chạy:** Xem tại [STLink_SWD_Debug/README.md](./STLink_SWD_Debug/Project/README.md)