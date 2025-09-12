/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @Date: 2025-02-19 09:42:35
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2025-04-05 07:49:12
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\HARDWARE\IO\GPIO.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "GPIO.h"
#include "at32f435_437_conf.h"
#include "at32f435_437_gpio.h"
#include "at32f435_437_crm.h"
#include "at32f435_437_tmr.h"


static void Timer_init(int ms)
{
    crm_periph_clock_enable(CRM_TMR10_PERIPH_CLOCK, TRUE);

    tmr_base_init(TMR10, ms * 10, 14400); // 假设时钟频率为 144MHz，PSC=14400 for 10kHz
    tmr_cnt_dir_set(TMR10, TMR_COUNT_UP);
    tmr_interrupt_enable(TMR10, TMR_OVF_INT, TRUE);
    tmr_counter_enable(TMR10, TRUE);

    nvic_irq_enable(TMR1_OVF_TMR10_IRQn, 0, 3);
}

void CAM_GPIO_INIT()
{
    crm_periph_clock_enable(CRM_GPIOG_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOF_PERIPH_CLOCK, TRUE);

    gpio_init_type gpio_init_struct;
    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;

    gpio_init_struct.gpio_pins = GPIO_PINS_0 | GPIO_PINS_1;
    gpio_init(GPIOG, &gpio_init_struct);

    gpio_init_struct.gpio_pins = GPIO_PINS_15;
    gpio_init(GPIOF, &gpio_init_struct);

    gpio_bits_reset(GPIOG, GPIO_PINS_0 | GPIO_PINS_1);
    gpio_bits_reset(GPIOF, GPIO_PINS_15);

    Timer_init(1500);
}

static int sec=0;
static int ReqSec=0;
void TMR1_OVF_TMR10_IRQHandler()
{
    if (tmr_flag_get(TMR10, TMR_OVF_FLAG) != RESET)
    {
        tmr_flag_clear(TMR10, TMR_OVF_FLAG);

        if (sec >= ReqSec)
        {
            gpio_bits_reset(GPIOG, GPIO_PINS_0 | GPIO_PINS_1);
            gpio_bits_reset(GPIOF, GPIO_PINS_15);
        }
        sec++;
    }
}
}

void GPIO_CAM_REC_IO_Ctrl()
{
    GPIOG->ODR |= GPIO_ODR_ODR_1;
		sec=0;
		ReqSec=150;
    
}

void GPIO_CAM_SyncLED_IO_Ctrl()
{
    GPIOF->ODR |= GPIO_ODR_ODR_15;
		sec=0;
		ReqSec=5;
}

void GPIO_CAM_Wifi_IO_Ctrl()
{
    GPIO_SetBits(GPIOG,GPIO_Pin_0);
		sec=0;
    ReqSec=500;
}