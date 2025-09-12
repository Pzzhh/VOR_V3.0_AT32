/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-24 14:47:04
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @LastEditTime: 2024-08-08 11:08:14
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\HARDWARE\SLAVE\Slave1_IO.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "Slave1_IO.h"
#include "stm32f4xx.h"

void Slave1_IO_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    GPIOA->MODER &= ~GPIO_MODER_MODER4;

    GPIOA->MODER &= ~GPIO_MODER_MODER7; // svr_dir
    GPIOA->MODER &= ~GPIO_MODER_MODER3; // svr_on

    GPIOA->MODER |= GPIO_MODER_MODER7_0; // alm
    GPIOA->MODER |= GPIO_MODER_MODER3_0; // alm
}

void Slave1_En_IO(uint8_t state)
{
    uint16_t *const gpio_reg = (uint16_t *)&GPIOA->BSRRL;
    const uint16_t gpio_pin = GPIO_Pin_3;
    if (state)
    {
        gpio_reg[0] = gpio_pin;
    }
    else
    {
        gpio_reg[1] = gpio_pin;
    }
}

void Slave1_Dir_IO(uint8_t state)
{
    uint16_t *const gpio_reg = (uint16_t *)&GPIOA->BSRRL;
    const uint16_t gpio_pin = GPIO_Pin_7;
    if (!state)
    {
        gpio_reg[0] = gpio_pin;
    }
    else
    {
        gpio_reg[1] = gpio_pin;
    }
}