/**
 * Chương trình gửi chuỗi "STM32 UART OK\r\n" qua USART1 (PA9) mỗi giây.
 * Cấu hình cho clock mặc định HSI 8MHz.
 * Baud rate: 9600
 */

//---- Các định nghĩa địa chỉ thanh ghi ----//

// RCC (Reset and Clock Control) - để bật clock cho các ngoại vi
#define RCC_BASE      0x40021000
#define RCC_APB2ENR   (*(volatile unsigned int*)(RCC_BASE + 0x18))

// GPIO Port A - để cấu hình chân PA9
#define GPIOA_BASE    0x40010800
#define GPIOA_CRH     (*(volatile unsigned int*)(GPIOA_BASE + 0x04))

// USART1 Peripheral
#define USART1_BASE   0x40013800
#define USART1_SR     (*(volatile unsigned int*)(USART1_BASE + 0x00)) // Status Register
#define USART1_DR     (*(volatile unsigned int*)(USART1_BASE + 0x04)) // Data Register
#define USART1_BRR    (*(volatile unsigned int*)(USART1_BASE + 0x08)) // Baud Rate Register
#define USART1_CR1    (*(volatile unsigned int*)(USART1_BASE + 0x0C)) // Control Register 1

//---- Các hàm chức năng ----//

/**
 * @brief Gửi một ký tự qua UART1.
 * @param c Ký tự cần gửi.
 */
void uart_send_char(char c) {
    // Chờ cho đến khi thanh ghi dữ liệu truyền trống (cờ TXE được set)
    // Cờ TXE nằm ở bit 7 của thanh ghi trạng thái USART1_SR
    while (!(USART1_SR & (1 << 7)));
    
    // Ghi ký tự vào thanh ghi dữ liệu để bắt đầu truyền
    USART1_DR = c;
}

/**
 * @brief Gửi một chuỗi ký tự qua UART1.
 * @param str Chuỗi cần gửi.
 */
void uart_send_string(const char* str) {
    while (*str) {
        uart_send_char(*str++);
    }
}

/**
 * @brief Hàm delay đơn giản bằng cách lãng phí chu kỳ CPU.
 * @param count Số chu kỳ để chờ.
 */
void delay(volatile unsigned int count) {
    while (count--);
}

//---- Chương trình chính ----//
int main(void) {
    // 1. Bật Clock cho các ngoại vi cần thiết
    // Bit 2: IOPAEN - Bật clock cho GPIO Port A
    // Bit 14: USART1EN - Bật clock cho USART1
    // Bit 0: AFIOEN - Bật clock cho Alternate Function I/O (cần cho UART)
    RCC_APB2ENR |= (1 << 2) | (1 << 14) | (1 << 0);

    // 2. Cấu hình chân PA9 (TX của USART1)
    // Chân PA9 được điều khiển bởi các bit trong thanh ghi GPIOA_CRH (Control Register High)
    // Xóa 4 bit cấu hình cũ của chân PA9 (bit 7, 6, 5, 4)
    GPIOA_CRH &= ~(0b1111 << 4); 
    // Thiết lập PA9: 
    // MODE9[1:0] = 11 (Output mode, max speed 50 MHz)
    // CNF9[1:0]  = 10 (Alternate function output Push-pull)
    // Giá trị cần ghi vào 4 bit là 1011b, tương đương 0xB
    GPIOA_CRH |= (0b1011 << 4);

    // 3. Cấu hình thông số cho USART1
    // Tắt USART để cấu hình
    USART1_CR1 &= ~(1 << 13); // Xóa bit UE (USART Enable)

    // Thiết lập Baud Rate = 9600 với PCLK2 = 8MHz
    // BRR = F_clk / (16 * BAUD) = 8,000,000 / (16 * 9600) = 52.083
    // Mantissa (phần nguyên) = 52 (0x34)
    // Fraction (phần thập phân) = round(0.083 * 16) = 1 (0x1)
    // ==> BRR = 0x341
    USART1_BRR = 0x341;

    // Bật bộ truyền (TE - Transmitter Enable) và bật lại USART (UE - USART Enable)
    // Bit 3: TE
    // Bit 13: UE
    USART1_CR1 |= (1 << 3) | (1 << 13);

    // Vòng lặp vô tận
    while (1) {
        uart_send_string("hellokitty\r\n");
        // Chờ khoảng 1 giây
        delay(800000); // Giá trị này có thể cần điều chỉnh cho phù hợp
    }

    return 0; // Dòng này sẽ không bao giờ được thực thi
}