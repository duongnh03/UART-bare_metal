//---- Register address definitions ----//

// RCC (Reset and Clock Control) - to enable clocks for peripherals
#define RCC_BASE      0x40021000
#define RCC_APB2ENR   (*(volatile unsigned int*)(RCC_BASE + 0x18))

// GPIO Port A - to configure pin PA9
#define GPIOA_BASE    0x40010800
#define GPIOA_CRH     (*(volatile unsigned int*)(GPIOA_BASE + 0x04))

// USART1 Peripheral
#define USART1_BASE   0x40013800
#define USART1_SR     (*(volatile unsigned int*)(USART1_BASE + 0x00)) // Status Register
#define USART1_DR     (*(volatile unsigned int*)(USART1_BASE + 0x04)) // Data Register
#define USART1_BRR    (*(volatile unsigned int*)(USART1_BASE + 0x08)) // Baud Rate Register
#define USART1_CR1    (*(volatile unsigned int*)(USART1_BASE + 0x0C)) // Control Register 1

//---- Helper functions ----//

/**
 * @brief Sends a single character via UART1.
 * @param c The character to be sent.
 */
void uart_send_char(char c) {
    // Wait until the transmit data register is empty (TXE flag is set)
    // The TXE flag is at bit 7 of the USART1_SR status register
    while (!(USART1_SR & (1 << 7)));
    
    // Write the character to the data register to start transmission
    USART1_DR = c;
}

/**
 * @brief Sends a string of characters via UART1.
 * @param str The string to be sent.
 */
void uart_send_string(const char* str) {
    while (*str) {
        uart_send_char(*str++);
    }
}

/**
 * @brief Simple delay function by wasting CPU cycles.
 * @param count The number of cycles to wait.
 */
void delay(volatile unsigned int count) {
    while (count--);
}

//---- Main program ----//
int main(void) {
    // 1. Enable Clocks for necessary peripherals
    // Bit 2: IOPAEN - Enable clock for GPIO Port A
    // Bit 14: USART1EN - Enable clock for USART1
    // Bit 0: AFIOEN - Enable clock for Alternate Function I/O (required for UART)
    RCC_APB2ENR |= (1 << 2) | (1 << 14) | (1 << 0);

    // 2. Configure pin PA9 (TX of USART1)
    // Pin PA9 is controlled by bits in the GPIOA_CRH (Control Register High) register
    // Clear the old 4 configuration bits for pin PA9 (bits 7, 6, 5, 4)
    GPIOA_CRH &= ~(0b1111 << 4); 
    // Set up PA9: 
    // MODE9[1:0] = 11 (Output mode, max speed 50 MHz)
    // CNF9[1:0]  = 10 (Alternate function output Push-pull)
    // The value to write to the 4 bits is 1011b, equivalent to 0xB
    GPIOA_CRH |= (0b1011 << 4);

    // 3. Configure USART1 parameters
    // Disable USART to configure it
    USART1_CR1 &= ~(1 << 13); // Clear the UE bit (USART Enable)

    // Set Baud Rate = 9600 with PCLK2 = 8MHz
    // BRR = F_clk / (16 * BAUD) = 8,000,000 / (16 * 9600) = 52.083
    // Mantissa (integer part) = 52 (0x34)
    // Fraction (decimal part) = round(0.083 * 16) = 1 (0x1)
    // ==> BRR = 0x341
    USART1_BRR = 0x341;

    // Enable the transmitter (TE - Transmitter Enable) and re-enable USART (UE - USART Enable)
    // Bit 3: TE
    // Bit 13: UE
    USART1_CR1 |= (1 << 3) | (1 << 13);

    // Infinite loop
    while (1) {
        uart_send_string("hellokitty\r\n");
        // Wait for about 1 second
        delay(800000); // This value may need to be adjusted for accuracy
    }

    return 0; // This line will never be executed
}