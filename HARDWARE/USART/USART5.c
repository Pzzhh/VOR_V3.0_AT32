/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-30 14:37:06
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2025-09-23 05:01:19
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\HARDWARE\USART\USART5.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "USART5.h"
#include "at32f435_437.h"

static void (*Interrupt_handle)(uint8_t dr);

void USART5_INIT(
    uint32_t boundrate,
    void (*handle)(uint8_t dr))
{
    gpio_init_type gpio_init_struct;

    /* enable the USART5 and gpio clock */
    crm_periph_clock_enable(CRM_UART5_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);

    gpio_default_para_init(&gpio_init_struct);

    /* configure the USART5 tx, rx pin */
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_pins = GPIO_PINS_12;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init(GPIOC, &gpio_init_struct);

    gpio_init_struct.gpio_pins = GPIO_PINS_2;
    gpio_init(GPIOD, &gpio_init_struct);

    gpio_pin_mux_config(GPIOC, GPIO_PINS_SOURCE12, GPIO_MUX_8);
    gpio_pin_mux_config(GPIOD, GPIO_PINS_SOURCE2, GPIO_MUX_8);

    //nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(UART5_IRQn, 0, 0);

    /* configure UART5 param */
    usart_init(UART5, boundrate, USART_DATA_8BITS, USART_STOP_1_BIT);
    usart_parity_selection_config(UART5, USART_PARITY_NONE);
    usart_transmitter_enable(UART5, TRUE);
    usart_receiver_enable(UART5, TRUE);
    usart_enable(UART5, TRUE);

    usart_interrupt_enable(UART5, USART_RDBF_INT, TRUE);
}

void UART5_IRQHandler(void) // 串口5中断服务程序
{
    if (usart_flag_get(UART5, USART_RDBF_FLAG) != RESET) // 接收中断(接收到的数据必须??0x0d 0x0a结尾)
    {
        uint8_t dr = usart_data_receive(UART5);
        if (Interrupt_handle != 0)
            Interrupt_handle(dr);
        usart_flag_clear(UART5, USART_RDBF_FLAG);
    }
}