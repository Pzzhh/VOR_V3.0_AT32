/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-24 14:44:19
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @LastEditTime: 2024-08-05 11:33:37
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\implement\Slave_Vor_Ctrl.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stm32f4xx.h"
#include "Slave_Zero_ctrl.h"
#include "motor_para_cal.h"
#include "math.h"
#include "delay.h"
#include "../HARDWARE/SLAVE/Slave1_IO.h"
#include "../HARDWARE/SLAVE/Slave1_Timer.h"
#include "../HARDWARE/IR_Sense/IR.h"

typedef struct
{
    enum
    {
        running,
        back,
        end,
    } state;
    uint32_t Tick;
    float vel;
    Speed_Cal_Struct back;
} Zero_Machine_parameter;
Zero_Machine_parameter zero_para;

void Zero_Back_init(float len, float sps, float accMs);

static uint8_t motor_set(void)
{
    float speed = 0;
    if (IR_IO_Get())
    {
        Slave1_Set_Encode(0);
        zero_para.state = back;
        Zero_Back_init(140, 30, 1);
        tim_f_set(angle_step * 0);
    }
    if (zero_para.state == running)
    {
        zero_para.back.motor.ms_esccape++;
        motor_speed_cal(0, &speed, &zero_para.back);
        tim_f_set(angle_step * speed);
    }
    if (zero_para.state == back)
    {
			 zero_para.back.motor.ms_esccape++;
        motor_speed_cal(0, &speed, &zero_para.back);
        tim_f_set(angle_step * speed);
    }
    return 0;
}
static uint8_t Zero_handler(void)
{
    uint8_t res = 0;
    res = motor_set();
    if (res)
    {
        zero_para.state = end;
        return 1; // end turn off tim4
    }
    return 0;
}

uint8_t Zero_Machine_Init(float vel, uint32_t MillSec)
{
    float speed;
#if 0
    Slave1_Set_Machine_Cb(CONT_handler);
    zero_para.Tick = 0;
    zero_para.state = running;
    zero_para.MillSecReq = 2000;
#else
    IR_IO_Init();
    Slave1_Set_Machine_Cb(Zero_handler);
    Zero_Back_init(360, 30, 1);
#endif
    return 1;
}

void Zero_Back_init(float len, float sps, float accMs)
{
    float speed;
    zero_para.back.motor.ms_esccape = 0;
    zero_para.back.acc_time = accMs;
    zero_para.back.sps = sps;
    zero_para.back.dir = 1;
    zero_para.back.lenght = len;
    motor_speed_cal(1, &speed, &zero_para.back);
}
