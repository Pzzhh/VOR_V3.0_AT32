/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-13 11:18:33
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2025-09-11 16:12:02
 * @FilePath: \USERd:\workfile\项目3 vor\software\vor-pjt\template\HARDWARE\CAN\can.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/**
 ****************************************************************************************************
 * @file        can.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-11
 * @brief       CAN 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20220111
 * 第一次发布
 *
 ****************************************************************************************************
 */

// AT32F435 适配
#include "at32f435_437_conf.h"
#include "at32f435_437_can.h"
#include "at32f435_437_gpio.h"
#include "at32f435_437_crm.h"
#include "../HARDWARE/CAN/can.h"

void (*can_rx_handler)(can_rx_message_type *msg);

// #include "./BSP/LED/led.h"
// #include "./SYSTEM/delay/delay.h"
// #include "./SYSTEM/usart/usart.h"


/**
 * @brief       AT32F435 CAN初始化（适配模板）
 * @param       tsjw    : 重新同步跳跃时间单元.范围: 1~3;
 * @param       tbs2    : 时间段2的时间单元.范围: 1~8;
 * @param       tbs1    : 时间段1的时间单元.范围: 1~16;
 * @param       brp     : 波特率分频器.范围: 1~1024;
 * @param       mode    : 0,普通模式;1,回环模式;
 * @retval      0,  初始化成功; 其他, 初始化失败;
 * @note        需根据实际硬件连接修改 GPIO/CRM 配置
 */
uint8_t can_init(uint8_t tsjw, uint8_t tbs2, uint8_t tbs1, uint16_t brp, uint8_t mode)
{
    // // 1. 使能外设时钟
    // crm_periph_clock_enable(CRM_CAN1_PERIPH_CLOCK, TRUE);
    // crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE); // 假设用PB8/PB9做CAN

    // // 2. 配置GPIO为复用功能
    // gpio_init_type gpio_init_struct;
    // gpio_default_para_init(&gpio_init_struct);
    // gpio_init_struct.gpio_pins = GPIO_PINS_8 | GPIO_PINS_9;
    // gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
    // gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    // gpio_init_struct.gpio_pull = GPIO_PULL_UP;
    // gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    // gpio_init(GPIOB, &gpio_init_struct);

    // gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE8, GPIO_MUX_9); // CAN_RX
    // gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE9, GPIO_MUX_9); // CAN_TX

    // // 3. CAN 参数配置
    // can_init_type can_init_struct;
    // can_default_para_init(&can_init_struct);
    // can_init_struct.mode = (mode == 0) ? CAN_MODE_COMMUNICATE : CAN_MODE_LOOPBACK;
    // can_init_struct.ttcm = FALSE;
    // can_init_struct.abom = FALSE;
    // can_init_struct.awum = FALSE;
    // can_init_struct.nart = TRUE;
    // can_init_struct.rflm = FALSE;
    // can_init_struct.txfp = FALSE;
    // can_init_struct.sjw = tsjw;
    // can_init_struct.bs1 = tbs1;
    // can_init_struct.bs2 = tbs2;
    // can_init_struct.divider = brp;
    // if (can_init(CAN1, &can_init_struct) != SUCCESS)
    //     return 1;

    // // 4. 过滤器配置（不过滤任何ID）
    // can_filter_init_type can_filter_init_struct;
    // can_filter_init_struct.filter_activate_enable = TRUE;
    // can_filter_init_struct.filter_mode = CAN_FILTER_MODE_IDMASK;
    // can_filter_init_struct.filter_fifo = CAN_FILTER_FIFO0;
    // can_filter_init_struct.filter_number = 0;
    // can_filter_init_struct.filter_bit = CAN_FILTER_32BIT;
    // can_filter_init_struct.filter_id_high = 0;
    // can_filter_init_struct.filter_id_low = 0;
    // can_filter_init_struct.filter_mask_id_high = 0;
    // can_filter_init_struct.filter_mask_id_low = 0;
    // can_filter_init(&can_filter_init_struct);

    // // 5. 中断配置（如需）
    // can_interrupt_enable(CAN1, CAN_RF0MIEN_INT, TRUE);
    // // nvic_irq_enable(CAN1_RX0_IRQn, 2, 0); // 需根据实际中断号配置

    return 0;
}

/**
 * @brief       CAN发送一条消息
 * @param       id      : 标准ID(11位)/扩展ID(11位+18位)
 * @param       ide     : 0,标准帧;1,扩展帧
 * @param       rtr     : 0,数据帧;1,远程帧
 * @param       len     : 要发送的数据长度(固定为8个字节, 在时间触发模式下, 有效数据为6个字节)
 * @param       data    : 数据指针
 * @retval      本次发送数据所用的邮箱号
 *   @arg       0 ~ 3   : 邮箱编号
 *   @arg       0XFF    : 无有效邮箱(数据发送失败)
 */

uint8_t can_tx_msg(uint32_t id, uint8_t ide, uint8_t rtr, uint8_t len, uint8_t *data)
{
    can_tx_message_type tx_msg;
    tx_msg.standard_id = (ide == 0) ? id : 0;
    tx_msg.extended_id = (ide == 1) ? id : 0;
    tx_msg.id_type = (ide == 0) ? CAN_ID_STANDARD : CAN_ID_EXTENDED;
    tx_msg.frame_type = (rtr == 0) ? CAN_TFT_DATA : CAN_TFT_REMOTE;
    tx_msg.dlc = len;
    for (uint8_t i = 0; i < len && i < 8; i++)
        tx_msg.data[i] = data[i];
    // if (can_message_transmit(CAN1, &tx_msg) == CAN_TMD_NOEMPTY)
    //     return 0xFF; // 无空邮箱
    return 0; // 发送成功
}

/**
 * @brief       获取CAN发送状态
 * @param       mbox    : 邮箱编号 (0 ~ 2)
 * @retval      发送状态
 *   @arg       0X00    : 挂起
 *   @arg       0X05    : 发送失败
 *   @arg       0X07    : 发送成功
 */
// AT32库不需要单独查询邮箱状态，发送API已返回状态

/**
 * @brief       得到在FIFO0/FIFO1中接收到的报文个数
 * @param       fifox   : 0, FIFO0; 1, FIFO1;
 * @retval      FIFO0 / FIFO1中报文的个数
 */
uint8_t can_pend_msg(uint8_t fifox)
{
    if (fifox == 0)
        return can_receive_message_pending_get(CAN1, CAN_RX_FIFO0);
    else
        return can_receive_message_pending_get(CAN1, CAN_RX_FIFO1);
}

/**
 * @brief       CAN接收一条消息
 * @param       fifox   : FIFO编号 0, FIFO0; 1, FIFO1;
 * @param       id      : 标准ID(11位)/扩展ID(11位+18位)
 * @param       ide     : 0,标准帧;1,扩展帧
 * @param       rtr     : 0,数据帧;1,远程帧
 * @param       len     : 接收到的数据长度(一般为8个字节)
 * @param       data    : 数据缓冲区
 * @retval      无
 */

void can_rx_msg(uint8_t fifox, uint32_t *id, uint8_t *ide, uint8_t *rtr, uint8_t *len, uint8_t *data)
{
    can_rx_message_type rx_msg;
    // if (can_message_receive(CAN1, fifox == 0 ? CAN_RX_FIFO0 : CAN_RX_FIFO1, &rx_msg) == SUCCESS)
    // {
    //     *ide = (rx_msg.id_type == CAN_ID_STANDARD) ? 0 : 1;
    //     *id = (rx_msg.id_type == CAN_ID_STANDARD) ? rx_msg.standard_id : rx_msg.extend_id;
    //     *rtr = (rx_msg.frame_type == CAN_TFT_DATA) ? 0 : 1;
    //     *len = rx_msg.dlc;
    //     for (uint8_t i = 0; i < rx_msg.dlc && i < 8; i++)
    //         data[i] = rx_msg.data[i];
    // }
}

#if CAN_RX0_INT_ENABLE /* 使能RX0中断 */

/**
 * @brief       CAN RX0 中断服务函数
 *   @note      处理CAN FIFO0的接收中断
 * @param       无
 * @retval      无
 */
void CAN1_RX0_IRQHandler(void)
{
    // can_message_type msg = {0};
    // can_rx_msg(0, &msg.standard_id, (uint8_t *)&msg.id_type, (uint8_t *)&msg.frame_type, &msg.dlc, msg.data);
    // if (can_rx_handler != 0)
    //     can_rx_handler(&msg);
}

#endif

/**
 * @brief       CAN 发送一组数据
 *   @note      发送格式固定为: 标准ID, 数据帧
 * @param       id      : 标准ID(11位)
 * @retval      发送状态 0, 成功; 1, 失败;
 */
uint8_t can_send_msg(uint32_t id, uint8_t *msg, uint8_t len)
{
    return can_tx_msg(id, 0, 0, len, msg);
}

/**
 * @brief       CAN 接收数据查询
 *   @note      接收数据格式固定为: 标准ID, 数据帧
 * @param       id      : 要查询的 标准ID(11位)
 * @param       buf     : 数据缓存区
 * @retval      接收结果
 *   @arg       0   , 无数据被接收到;
 *   @arg       其他, 接收的数据长度
 */
uint8_t can_receive_msg(uint32_t id, uint8_t *buf)
{
    uint32_t rid = 0;
    uint8_t ide, rtr, len = 0;
    if (can_pend_msg(0) == 0)
        return 0;
    can_rx_msg(0, &rid, &ide, &rtr, &len, buf);
    if (rid != id || ide != 0 || rtr != 0)
        len = 0;
    return len;
}
