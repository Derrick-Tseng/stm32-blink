#include "stm32f4xx.h"
#include "stdio.h"
#include "stdarg.h"
#include "ring.h"

static void delay(volatile uint32_t n) { while (n--) { } }   /* 粗略延遲 */

// 寫入時進的是 TDR（發送），讀取時讀的是 RDR（接收）
// TXE = 1：TDR 空了，可以寫下一個字元
// TXE = 0：上一個字元還在 TDR 裡等，不能寫
// 不需要自己清 TXE，寫 DR 時硬體會自動清
static void uart_putc(char c){
    while(!(USART2->SR & USART_SR_TXE)){} // wait for TDR
    USART2->DR = c;
}


static void uart_puts(const char *s){
    while (*s) {            /* 還沒遇到 '\0' */
        uart_putc(*s++);    /* 送出目前這個字元，再把指標往後移一格 */
    }
}

static void uart_putu(uint32_t n){
    char buf[10];                   /* uint32_t 最多 10 位數 */
    int i = 0;

    do {
        buf[i++] = n % 10 + '0';    /* 從個位數開始取，存進來的順序是反的 */
        n /= 10;
    } while (n);                    /* 用 do-while：n 是 0 時也會存一個 '0' */

    while (i > 0) {
        uart_putc(buf[--i]);        /* 倒著送出，變回正確順序 */
    }
}

__attribute__((format(printf, 1, 2)))
static void uart_printf(const char *fmt, ...){
    char buf[128];
    va_list ap;  // va_list像是一個指向variadic的游標
    va_start(ap, fmt); // start end 成對
    vsnprintf(buf, sizeof(buf), fmt, ap); // v版本接收va_list
    va_end(ap);
    uart_puts(buf);
}


int main(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   /* 開 GPIOA 的時脈 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;   /* 開 GPIOC 的時脈 */
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;  /* 開 USART2 的時脈 */
    (void)RCC->AHB1ENR;                    /* 讀回一次，確保時脈真的開了 */

    // PB8 PB9
    GPIOA->MODER &= ~((3U << (2 * 2)) | (3U << (3 * 2)));
    GPIOA->MODER |=  ((2U << (2 * 2)) | (2U << (3 * 2))); // Set to AF mode

    // 板子上的燈
    GPIOA->MODER &= ~(3U << (5 * 2));      /* PA5 的 MODER[11:10] 先清 0 */
    GPIOA->MODER |=  (1U << (5 * 2));      /* 再設成 01 = 輸出 */

    GPIOA->AFR[0] &= ~((0xFU << (2 * 4)) | (0xFU << (3 * 4)));
    GPIOA->AFR[0] |= ((7U << (2 * 4)) | (7U << (3 * 4)));

    // PC13 板子上的按鈕
    GPIOC->MODER &= ~(3U << (13 * 2)); // clear and set to 00 for input
    GPIOC->PUPDR &= ~(3U << (13 * 2));  // clear
    GPIOC->PUPDR |=  (1U << (13 * 2));  // set to pull up (01)


    // baud rate: 16MHz HSI / 115200 ~= 139
    USART2->BRR = 139;

    // enable usart, transmitter, and receiver
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;


    static uint32_t cnt = 0;

    long tick = 1;
    uart_printf("tick=%d\r\n", tick);

    for (;;) {
        if(USART2->SR & USART_SR_RXNE){
            char c = USART2->DR;
            uart_putc(c);
            if(c == '\r'){
                uart_putc('\n');                       /* Enter 只送 \r，補 \n 才會換行 */
            }
            else if(c == 'r'){
                uart_puts("\r\nB1 = ");
                uart_putu((GPIOC->IDR >> 13) & 1U);    /* 放開 1、按下 0 */
                uart_puts("\r\n");
            }
            else if(c == 'l'){
                if(GPIOA->ODR & (1U << 5)){            /* BSRR 是唯寫，讀回永遠是 0，要讀 ODR */
                    GPIOA->BSRR = (1U << (5 +16)); // 板子LED滅
                }
                else{
                    GPIOA->BSRR = (1U << 5); // 板子LED亮
                }
            }
        }
    }
}
