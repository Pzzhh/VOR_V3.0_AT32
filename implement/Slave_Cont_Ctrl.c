/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-24 14:44:19
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @LastEditTime: 2024-08-08 11:05:33
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\implement\Slave_Vor_Ctrl.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stm32f4xx.h"
#include "Slave_Cont_Ctrl.h"
#include "motor_para_cal.h"
#include "math.h"
#include "delay.h"
#include "../HARDWARE/SLAVE/Slave1_IO.h"
#include "../HARDWARE/SLAVE/Slave1_Timer.h"

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
    // Task_Parameter_Struct info;
    uint32_t CurrentMillSec;
    uint32_t MillSecReq;
    Speed_Cal_Struct back;
} Cont_Machine_parameter;
Cont_Machine_parameter cont_para;
#define Pi 3.1415926
float speeds = 0;

void Cont_Back_init(float tragetPos, float sps, float accMs);

static uint8_t motor_set(void)
{
	static int DscTimer;
    uint8_t res;
    if (cont_para.state == running)
    {
        cont_para.CurrentMillSec = cont_para.Tick++;
			float factor =  cont_para.CurrentMillSec/1000.0f;
			factor=(factor>1)?1:factor;
        tim_f_sin_set(angle_step * cont_para.vel*factor);
        if (cont_para.CurrentMillSec >= cont_para.MillSecReq)
        {
//            float pos = Slave1_Get_Encode_Angle();
//            float traget = pos - (((int32_t)pos / 360)) * 360 - 180;
//            if (traget > 0)
//            {
//                traget = 360;
//            }
//            else
//                traget = 0;
//            tim_f_sin_set(angle_step * 0);
//            Cont_Back_init(traget, 30, 5);
					DscTimer=1000;
            cont_para.state = back;
            return 0;
        }
    }
    if (cont_para.state == back)
    {
//        cont_para.back.motor.ms_esccape++;
//        res = motor_speed_cal(0, &speeds, &cont_para.back);
//        if (res == 0)
//        {
//            tim_f_sin_set(0);
//            return 1;
//        }
//        tim_f_sin_set(angle_step * speeds);
			DscTimer-=1;
			 tim_f_sin_set(angle_step * cont_para.vel*DscTimer/1000.0f);
			if(DscTimer<=0)
			{
					 tim_f_sin_set(0);
				return 1;
			}
    }
    return 0;
}
static uint8_t CONT_handler(void)
{
    uint8_t res = 0;
    res = motor_set();
    if (res)
    {
        cont_para.state = end;
        return 1; // end turn off tim4
    }
    return 0;
}

uint8_t CONT_Machine_Init(float vel, uint32_t MillSec)
{
#if 0
    Slave1_Set_Machine_Cb(CONT_handler);
    cont_para.Tick = 0;
    cont_para.state = running;
    cont_para.MillSecReq = 2000;
#else
    if (vel > 360 || vel < -360)
        return 0;
    cont_para.vel = vel;
    cont_para.vel = vel;
    cont_para.MillSecReq = MillSec;
    Slave1_Step_Generator_Init(50000, 10);
    Slave1_Set_Machine_Cb(CONT_handler);
    cont_para.Tick = 0;
    cont_para.state = running;
#endif
    return 1;
}

uint8_t Cont_Machine_Stop(void)
{
    cont_para.CurrentMillSec = 0;
    cont_para.state = back;
    return 0;
}
#define abs(n) ((n > 0) ? (n) : (-(n)))
void Cont_Back_init(float tragetPos, float sps, float accMs)
{
    float speed, pos = Slave1_Get_Encode_Angle();
    cont_para.back.motor.ms_esccape = 0;
    cont_para.back.acc_time = accMs;
    cont_para.back.sps = sps;
    if (tragetPos > pos)
        cont_para.back.dir = 0;
    else
        cont_para.back.dir = 1;

    cont_para.back.lenght = abs(tragetPos - pos);
    motor_speed_cal(1, &speed, &cont_para.back);
}

uint8_t Cont_Machine_Get_Count(uint32_t *MillSecReq, uint32_t *CurrentMillSec)
{
    if (cont_para.state == end)
        return 0;
    if (MillSecReq != 0)
        *MillSecReq = cont_para.MillSecReq;
    if (CurrentMillSec != 0)
        *CurrentMillSec = cont_para.CurrentMillSec;
    return 1;
}
