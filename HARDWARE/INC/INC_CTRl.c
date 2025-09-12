/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @Date: 2024-08-05 14:28:29
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2024-09-05 18:17:30
 * @FilePath: \ubuntud:\workfile\项目3 vor\software\VOR_V2.0\HARDWARE\INC\INC_CTRl.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-08-05 14:28:29
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @LastEditTime: 2024-08-08 11:07:07
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\HARDWARE\INC\INC_CTRl.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "INC_CTRl.h"
#include "stm32f4xx.h"
// 0:stop 1:up 2:down
void INC_IO_Set(uint8_t state)
{
    enum state
    {
        stop,
        up,
        down
    };
    volatile uint16_t *const gpio_reg = (uint16_t *)&GPIOF->BSRRL;
    const uint16_t gpio_pinUP = GPIO_Pin_14;
    const uint16_t gpio_pinDown = GPIO_Pin_13;
		#if 1
    if (state == down)
    {
        gpio_reg[0] = gpio_pinDown;
        gpio_reg[1] = gpio_pinUP;
    }
    else if (state == up)
    {

        gpio_reg[1] = gpio_pinDown;
        gpio_reg[0] = gpio_pinUP;
    }
    else
    {
        uint16_t value = gpio_pinDown | gpio_pinUP;
        gpio_reg[1] = value;
    }
		#else
		 if (state == down)
    {
        gpio_reg[0] = gpio_pinDown;
        gpio_reg[1] = gpio_pinUP;
    }
    else if (state == up )
    {

        gpio_reg[1] = gpio_pinDown;
        gpio_reg[0] = gpio_pinUP;
    }
    else
    {
        uint16_t value = gpio_pinDown | gpio_pinUP;
        gpio_reg[1] = value;
    }
		#endif
}

void INC_IO_INIT()
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;

    GPIOF->MODER &= ~GPIO_MODER_MODER13;
    GPIOF->MODER &= ~GPIO_MODER_MODER14;

    GPIOF->MODER |= GPIO_MODER_MODER13_0;
    GPIOF->MODER |= GPIO_MODER_MODER14_0;

    INC_IO_Set(0);
}
