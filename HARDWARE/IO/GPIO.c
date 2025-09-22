/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @Date: 2025-05-28 14:59:54
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2025-09-22 11:44:18
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\HARDWARE\IO\GPIO.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @Date: 2025-02-19 09:42:35
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2025-08-13 14:57:27
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\HARDWARE\IO\GPIO.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "GPIO.h"
#include "at32f435_437.h"

static void Timer_init(int ms)
{
  crm_periph_clock_enable(CRM_TMR10_PERIPH_CLOCK, TRUE);

  TMR10->div = 14400;
  TMR10->pr = ms * 10; // interval 10ms
  TMR10->cval = 1;
  TMR10->ctrl1_bit.tmren = TRUE;
  TMR10->iden_bit.ovfien = TRUE;
  TMR10->ists = 0;
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_2);
  nvic_irq_enable(TMR1_OVF_TMR10_IRQn, 0, 3);
  // GPIOF->ODR |= GPIO_ODR_ODR_15;
}

void CAM_GPIO_INIT()
{
  gpio_init_type gpio_init_struct;

  crm_periph_clock_enable(CRM_GPIOG_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOF_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);

  gpio_init_struct.gpio_mode = GPIO_MODE_OUTPUT;
  gpio_init_struct.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;

  gpio_init_struct.gpio_pins = GPIO_PINS_0;
  gpio_init(GPIOG, &gpio_init_struct);  // REC

  gpio_init_struct.gpio_pins = GPIO_PINS_1;
  gpio_init(GPIOG, &gpio_init_struct);  // WIFI

  gpio_init_struct.gpio_pins = GPIO_PINS_15;
  gpio_init(GPIOF, &gpio_init_struct); // SYNC LED

  gpio_init_struct.gpio_pins = GPIO_PINS_3;
  gpio_init(GPIOD, &gpio_init_struct);  // BEEP

  gpio_bits_reset(GPIOG, GPIO_PINS_0);  // REC
  gpio_bits_reset(GPIOG, GPIO_PINS_1);  // WIFI
  gpio_bits_reset(GPIOF, GPIO_PINS_15); // SYNC LED
  gpio_bits_set(GPIOD, GPIO_PINS_3);   // BEEP
  Timer_init(1500);
}

static unsigned sec = 0;
static unsigned int ReqSec = 0, BeepReqSec = 0;
void TMR1_OVF_TMR10_IRQHandler()
{
  TMR10->ists = 0;
  if (sec == 0)
  {
    ReqSec = 1;
    BeepReqSec = 1;
  }
  if (ReqSec != 0 && sec >= ReqSec)
  {
    gpio_bits_reset(GPIOG, GPIO_PINS_0);
    gpio_bits_reset(GPIOF, GPIO_PINS_15);
    ReqSec = 0;
    //				gpio_bits_reset(GPIOG, GPIO_PINS_1);
  }
  if (BeepReqSec != 0 && sec >= BeepReqSec)
  {
    gpio_bits_set(GPIOD, GPIO_PINS_3);
    BeepReqSec = 0;
    //				gpio_bits_reset(GPIOG, GPIO_PINS_1);
  }
  sec++;
}

void GPIO_CAM_REC_IO_Ctrl(int flag)
{
  if (flag)
    gpio_bits_set(GPIOG, GPIO_PINS_1);
  else
    gpio_bits_reset(GPIOG, GPIO_PINS_1);
}

void GPIO_CAM_SyncLED_IO_Ctrl()
{
  gpio_bits_set(GPIOF, GPIO_PINS_15);
  // sec=0;
  ReqSec = sec + 5;
}

void GPIO_CAM_Wifi_IO_Ctrl()
{
  gpio_bits_set(GPIOG, GPIO_PINS_0);
  // sec=0;
  ReqSec = sec + 500;
}

void GPIO_Beep_IO_Ctrl(uint32_t ms)
{
  if (ms > 10 * 1000)
    ms = 10 * 1000;
  ms /= 10;
  gpio_bits_reset(GPIOD, GPIO_PINS_3);
  // sec=0;
  BeepReqSec = sec + ms;
}