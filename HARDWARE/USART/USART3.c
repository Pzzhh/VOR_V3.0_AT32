/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-30 14:37:06
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @LastEditTime: 2024-08-02 18:28:20
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\HARDWARE\USART\USART5.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "USART3.h"
#include "at32f435_437_conf.h"
#include "at32f435_437_usart.h"
#include "at32f435_437_gpio.h"
#include "at32f435_437_crm.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

static void (*Interrupt_handle)(uint8_t dr);

void USART3_INIT(
    uint32_t boundrate,
    void (*handle)(uint8_t dr))
{
    // GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    Interrupt_handle = handle;

    // 1. 使能外设时钟
    crm_periph_clock_enable(CRM_USART3_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);

    // 2. 配置GPIO为复用功能
    gpio_init_type gpio_init_struct;
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_pins = GPIO_PINS_10 | GPIO_PINS_11;
    gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init(GPIOC, &gpio_init_struct);
    gpio_pin_mux_config(GPIOC, GPIO_PINS_SOURCE10, GPIO_MUX_7); // RX
    gpio_pin_mux_config(GPIOC, GPIO_PINS_SOURCE11, GPIO_MUX_7); // TX

    // 3. USART 参数配置
    usart_init_type usart_init_struct;
    usart_default_para_init(&usart_init_struct);
    usart_init_struct.baudrate = boundrate;
    usart_init_struct.data_bits = USART_DATA_8BITS;
    usart_init_struct.stop_bits = USART_STOP_1_BIT;
    usart_init_struct.parity = USART_PARITY_NONE;
    usart_init_struct.hardware_flow_control = USART_HARDWARE_FLOW_NONE;
    usart_init_struct.transmitter_enable = TRUE;
    usart_init_struct.receiver_enable = TRUE;
    usart_init(USART3, &usart_init_struct);

    // 4. 使能USART
    usart_enable(USART3, TRUE);

    // 5. 使能接收中断
    usart_interrupt_enable(USART3, USART_RDBF_INT, TRUE);
    nvic_irq_enable(USART3_IRQn, 0, 2);
}
static char printf_buf[100];
static uint16_t send_index, send_len;
uint8_t USART3_FIFO_Send(uint8_t *dr)
{
    if (send_index >= send_len)
    {
        send_index = 0;
        send_len = 0;
        return 0;
    }
    if (send_len)
    {
        *dr = printf_buf[send_index++];
    }
    return 1;
}

uint8_t USART3_Send_Package(char *string, uint16_t size)
{
    if (send_len)
        return 0;
    memcpy(printf_buf, string, size);
    send_len = size;
    usart_interrupt_enable(USART3, USART_TDBE_INT, TRUE); // 使能发送缓冲区空中断
    return 1;
}

uint8_t USART3_PRINTF(char *string, uint16_t size)
{
    if (send_len)
        return 0;
    memcpy(printf_buf, string, size);
    send_len = size;
    usart_interrupt_enable(USART3, USART_TDBE_INT, TRUE); // 使能发送缓冲区空中断
    return 1;
}

void USART3_IRQHandler(void)
{
    if (usart_flag_get(USART3, USART_RDBF_FLAG) != RESET)
    {
        uint8_t dr = usart_data_receive(USART3);
        if (Interrupt_handle != 0)
            Interrupt_handle(dr);
        usart_flag_clear(USART3, USART_RDBF_FLAG);
    }
    if (usart_flag_get(USART3, USART_TDBE_FLAG) != RESET)
    {
        uint8_t dr;
        if (USART3_FIFO_Send(&dr))
        {
            usart_data_transmit(USART3, dr);
        }
        else
        {
            usart_interrupt_enable(USART3, USART_TDBE_INT, FALSE);
        }
    }
    // 清除所有中断标志（如有需要）
}