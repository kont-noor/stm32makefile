#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

/* User defined function prototypes */
void GPIOA_Init(void);
void TIM2_CC1_Init(void);
void led_toggle(void);

int main(void)
{
    /* Initialize GPIOC PIN13 */
    GPIOA_Init();

    /* Initialize TIM2 Capture/Compare 1 in output compare mode */
    TIM2_CC1_Init();

    /* Toggle LED forever */
    while(1)
    {
        /* Do nothing, all happens in ISR */
    }
}

/* Initialize GPIOA */
void GPIOA_Init(void)
{
    /* Configuration info. for PORT
     * - Speed = 50MHz
     * - Push-pull output mode
     */
    GPIO_InitTypeDef gpioa_config = { GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3,
                                      GPIO_Speed_50MHz,
                                      GPIO_Mode_Out_PP };

    /* Enable PORT A clock */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    /* Configure PORTC PIN 13 */
    GPIO_Init(GPIOA, &gpioa_config);

    /* Turn off LED to start with */
    //GPIOA->BSRR = (uint32_t)1 << 1;
}

/* Toggle LED */
void led_toggle(void)
{
    static uint8_t led = 1;
    /* If PORTC BIT 13 clear, set it */
    if((GPIOA->ODR & (uint32_t)1<<led) == 0)
    {
        GPIOA->BSRR = (uint32_t)1 << led;
    }
    /* If PORTC BIT 13 set, clear it */
    else
    {
        GPIOA->BRR = (uint32_t)1 << led;
    }
	led ++;
	if (led > 3)
		led = 1;
}

/* Configure TIM2 Capture/Compare 1 to work in output compare mode
 * so that the red LED flashes at 1HZ (toggle every 0.5s) */
void TIM2_CC1_Init(void)
{
    /* Enable TIM2 clock */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    /* Enable TIM2 global interrupt */
    NVIC->ISER[0] |= 0x10000000;

    /* Frequency after prescalar = 72MHz/(7199+1) = 10KHz.
     * Compare register = 5000 so a compare match occurs every 0.5s.
     */
    TIM2->PSC = (uint16_t)7199;
    TIM2->CCR1 = (uint16_t)5000;

    /* Enable Capture/Compare 1 interrupt */
    TIM2->DIER |= (uint16_t)0x0002;

    /* Enable TIM2 counter (in upcount mode) */
    TIM2->CR1 |= (uint16_t)0x0001;
}

/* Timer 2 Interrupt Service Routine */
void TIM2_IRQHandler(void)
{
    /* Toggle LED if TIM2's Capture/Compare 1 interrupt occurred. */
    if(TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
    {
        led_toggle();
        /* Clear TIM2's Capture/Compare 1 interrupt pending bit. */
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
        /* Increment compare register by 5000 so next interrupt
         * occurs in 0.5s */
        TIM2->CCR1 += (uint16_t)1000;
    }

    /* ===== Other TIM2 interrupt types can go below ======
     * .........
     */
}
