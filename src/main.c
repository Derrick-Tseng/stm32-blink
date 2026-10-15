#include "stm32f4xx.h"

int main(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;      /* 打開 GPIOA 時脈 */
    (void)RCC->AHB1ENR;                        /* 讀回一次，確保時脈已生效 */

    GPIOA->MODER &= ~(3U << (5 * 2));          /* PA5 先清成 00 */
    GPIOA->MODER |=  (1U << (5 * 2));          /* 再設成 01 = 輸出 */

    for (;;) {
        GPIOA->ODR ^= (1U << 5);               /* 翻轉 LD2 */
        for (volatile uint32_t i = 0; i < 1000000; i++) { }
    }
}
