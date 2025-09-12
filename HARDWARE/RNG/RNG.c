
#include "stm32f4xx.h"
#include "RNG.h"
// 0:none 1:reach
uint8_t RNG_Generate(void)
{
    return RNG->DR;
}

void RNG_Generate_Init(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;
    RNG->CR |= RNG_CR_RNGEN;
}
