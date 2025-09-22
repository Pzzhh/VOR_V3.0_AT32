/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-30 14:37:06
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2024-09-06 14:26:22
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\HARDWARE\USART\USART5.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "USART6.h"
#include "at32f435_437.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"

static void (*Interrupt_handle)(uint8_t dr);
static uint8_t isinit=0;

void USART6_INIT(
    uint32_t boundrate,
    void (*handle)(uint8_t dr))
{
    // GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    Interrupt_handle = handle;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);  // 使能GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE); // 使能UART5时钟

    // 串口1对应引脚复用映射
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource9, GPIO_AF_USART6);  // GPIOA9复用为UART5
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_USART6); // GPIOA10复用为UART5

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;        //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;      // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 速度50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    // 推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      // 上拉
    GPIO_Init(GPIOG, &GPIO_InitStructure);            //

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;         // GPIOA9与GPIOA10
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;      // 复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 速度50MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;    // 推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;      // 上拉
    GPIO_Init(GPIOG, &GPIO_InitStructure);            // 初始化PA9，PA10

    // UART5 初始化设置
    USART_InitStructure.USART_BaudRate = boundrate;                                 // 波特率设置
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 收发模式
    USART_Init(USART6, &USART_InitStructure);                                       // 初始化串口1

    USART_Cmd(USART6, ENABLE); // 使能串口1

    // #if EN_UART5_RX
    USART_ITConfig(USART6, USART_IT_RXNE, ENABLE); // 开启相关中断
		USART_ITConfig(USART6, USART_IT_TXE, ENABLE); // 开启相关中断

    // UART5 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;         // 串口1中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // 子优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);                           // 根据指定的参数初始化VIC寄存器、
		isinit=1;
}
static char printf_buf[100];
static uint16_t send_index, send_len;
uint8_t USART6_FIFO_Send(uint8_t *dr)
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

uint8_t USART6_Send_Package(char *string, uint16_t size)
{
		if(isinit==0)
		 return 1;
    if (send_len)
        return 0;
    memcpy(printf_buf, string, size);
    send_len = size;
    USART_ITConfig(USART6, USART_IT_TXE, ENABLE); // 开启相关中断
    return 1;
}

void USART6_IRQHandler(void) // 串口1中断服务程序
{
    if (USART_GetITStatus(USART6, USART_IT_RXNE) != RESET) // 接收中断(接收到的数据必须??0x0d 0x0a结尾)
    {
        uint8_t dr = USART6->DR;
        if (Interrupt_handle != 0)
            Interrupt_handle(dr);
        USART_ClearITPendingBit(USART6, USART_IT_RXNE);
    }
    if (USART_GetITStatus(USART6, USART_IT_TXE))
    {
        uint8_t dr;
        if (USART6_FIFO_Send(&dr))
        {
            USART6->DR = dr;
        }
        else
        {
            USART_ITConfig(USART6, USART_IT_TXE, DISABLE);
        }
				 USART_ClearITPendingBit(USART6, USART_IT_TXE);
    }
//    USART6->SR = 0;
}