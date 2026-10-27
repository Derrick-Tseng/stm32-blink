#include "stm32f4xx.h"
#include "stdio.h"
#include "stdarg.h"
#include "ring.h"

static void delay(volatile uint32_t n) { while (n--) { } }   /* 粗略延遲 */
static ringbuf_t uart2_rx;


// DR(Data Register) 寫入時進的是 TDR（發送），讀取時讀的是 RDR（接收）
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

int uart_readByte(uint8_t *b){
    return rb_get(&uart2_rx, b) ? 1 : 0;
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

void USART2_IRQHandler(void){
    uint32_t sr = USART2->SR;
    uint8_t dr = (uint8_t)USART2->DR;

    // RXNE = Read data register Not Empty
    // RXNE=1 means there is new data
    if(sr & USART_SR_RXNE){
        rb_put(&uart2_rx, dr);
    }
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
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;

    // Enable NVIC
    NVIC_EnableIRQ(USART2_IRQn);

    // init ring buffer
    rb_init(&uart2_rx);

    static uint32_t cnt = 0;

    uint8_t c;
    for (;;) {
        while(uart_readByte(&c)){
            uart_putc(c);
        }
    }
}
