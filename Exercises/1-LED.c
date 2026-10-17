#include "stm32f4xx.h"

static void delay(volatile uint32_t n) { while (n--) { } }   /* 粗略延遲 */
volatile uint32_t now_delay = 100000;

int main(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;   /* 開 GPIOA 的時脈 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;      /* 開 GPIOB 的時脈 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;      /* 開 GPIOC 的時脈 */
    (void)RCC->AHB1ENR;                       /* 讀回一次，確保時脈真的開了 */

    // 紅綠燈 PB8 PB9
    GPIOB->MODER &= ~((3U << (8 * 2)) | (3U << (9 * 2)));
    GPIOB->MODER |=  ((1U << (8 * 2)) | (1U << (9 * 2)));

    // 板子上的燈 PA5
    GPIOA->MODER &= ~(3U << (5 * 2));      /* PA5 的 MODER[11:10] 先清 0 */
    GPIOA->MODER |=  (1U << (5 * 2));      /* 再設成 01 = 輸出 */

    // 板子上的按鈕
    GPIOC->PUPDR &= ~(3U << (13 * 2));
    GPIOC->PUPDR |=  (1U << (13 * 2));


    uint32_t last = 1;    /* 放開時是高電位 */
    for (;;) {
        uint32_t now = (GPIOC->IDR >> 13) & 1U;
        
        if (last == 1 && now == 0){
            
            if(now_delay == 500000){
                now_delay = 100000;
            }
            else{
                now_delay += 100000;
            }
        }
        last = now;

        GPIOB->BSRR = (1U << 8) | (1U << (9 +16)); /* LED1 亮、LED2 滅 */
        GPIOA->BSRR = (1U << (5 +16)); // 板子LED滅

        delay(now_delay);

        GPIOB->BSRR = (1U << (8 + 16)) | (1U << 9); /* LED1 滅、LED2 亮 */
        GPIOA->BSRR = (1U << (5 +16)); // 板子LED滅

        delay(now_delay);

        GPIOB->BSRR = (1U << (8 + 16)) | (1U << (9 + 16)); /* LED1 滅、LED2 滅 */
        GPIOA->BSRR = (1U << 5); // 板子LED亮

        delay(now_delay);
        
    }
}
