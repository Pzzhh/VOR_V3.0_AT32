#include "FAN.h"
#include "stm32f4xx.h"

void FAN_Init()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    TIM4->CCR1 = 0;

    GPIOB->MODER &= ~GPIO_MODER_MODER6;
    GPIOB->MODER |= GPIO_MODER_MODER6_1;

    GPIOB->AFR[0] |= 0x2 << (4 * (6)); // PA8 corresponds to AFR[1] (AFRH), 0th position

    // Set the prescaler value to have TIM4 counter clock = 1 MHz
    TIM4->PSC = 84 - 1; // APB2 clock is 84 MHz

    // Set the ARR value to have a PWM frequency of 1 kHz
    TIM4->ARR = 1000 - 1;

    // Set the CCR1 value to have a duty cycle of 50%
    TIM4->CCR1 = 500;

    // Configure the TIM4 channel 1 in PWM mode 1
    TIM4->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM4->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2;

    // Enable the output compare 1 preload register
    TIM4->CCMR1 |= TIM_CCMR1_OC1PE;

    // Enable the capture/compare 1 output
    TIM4->CCER |= TIM_CCER_CC1E;

    // Enable auto-reload preload
    TIM4->CR1 |= TIM_CR1_ARPE;

    // Enable the TIM4 main output
    TIM4->BDTR |= TIM_BDTR_MOE;

    // Enable the TIM4 counter
    TIM4->CR1 |= TIM_CR1_CEN;
}

void FAN_Ctrl(uint8_t pwn_precent)
{
    TIM4->CCR1 = pwn_precent*10;
}
