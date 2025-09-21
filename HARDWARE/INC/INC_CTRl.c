/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @Date: 2024-08-05 14:28:29
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2025-09-16 17:59:42
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
#include "at32f435_437.h"

#define INC_IO_UP GPIOF->odt_bit.odt14
#define INC_IO_DW GPIOF->odt_bit.odt13
// 0:stop 1:up 2:down
void INC_IO_Set(uint8_t state)
{

    enum state
    {
        stop,
        up,
        down
    };
    // volatile uint16_t *const gpio_reg = (uint16_t *)&GPIOF->BSRRL;
    // const uint16_t gpio_pinUP = GPIO_Pin_14;
    // const uint16_t gpio_pinDown = GPIO_Pin_13;
#if 1
    if (state == down)
    {
        INC_IO_UP = 0;
        INC_IO_DW = 1;
    }
    else if (state == up)
    {

        INC_IO_UP = 1;
        INC_IO_DW = 0;
    }
    else
    {
        INC_IO_UP = 0;
        INC_IO_DW = 0;
    }
#else
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
#endif
}

void INC_IO_INIT()
{
    CRM->ahben1_bit.gpiofen = 1;

    gpio_init_type gpio_init_struct;
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_pins = GPIO_PINS_13;
    gpio_init_struct.gpio_pull = GPIO_PULL_DOWN;
    gpio_init(GPIOF, &gpio_init_struct);

    gpio_init_struct.gpio_pins = GPIO_PINS_14;
    gpio_init(GPIOF, &gpio_init_struct);

    INC_IO_Set(0);
}
