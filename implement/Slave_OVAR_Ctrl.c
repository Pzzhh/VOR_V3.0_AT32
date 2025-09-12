/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-24 14:44:19
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @LastEditTime: 2024-08-05 14:08:45
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\implement\Slave_Vor_Ctrl.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stm32f4xx.h"
#include "Slave_OVAR_Ctrl.h"
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
} Ovar_Machine_parameter;
Ovar_Machine_parameter ovar_para;
#define Pi 3.1415926
//static float speeds = 0;

void OVAR_Back_init(float tragetPos, float sps, float accMs);

static uint8_t motor_set(void)
{
		static int DscTimer;
    if (ovar_para.state == running)
    {
        ovar_para.CurrentMillSec = ovar_para.Tick++;
			float factor =  ovar_para.CurrentMillSec/1000.0f;
			factor=(factor>1)?1:factor;
        tim_f_sin_set(angle_step * ovar_para.vel*factor);
        if (ovar_para.CurrentMillSec >= ovar_para.MillSecReq)
        {
					DscTimer=1000;
            ovar_para.state = back;
            return 0;
        }
    }
    if (ovar_para.state == back)
    {
				DscTimer-=1;
			 tim_f_sin_set(angle_step * ovar_para.vel*DscTimer/1000.0f);
			if(DscTimer<=0)
			{
					 tim_f_sin_set(0);
				return 1;
			}
    }
    return 0;
//    uint8_t res;
//    if (ovar_para.state == running)
//    {
//        ovar_para.CurrentMillSec = ovar_para.Tick++;
//        tim_f_sin_set(angle_step * ovar_para.vel);
//        if (ovar_para.CurrentMillSec >= ovar_para.MillSecReq)
//        {
//            float pos = Slave1_Get_Encode_Angle();
//            float traget = pos - (((int32_t)pos / 360)) * 360 - 180;
//            if (traget > 0)
//            {
//                traget = 360;
//            }
//            else
//                traget = 0;
//            tim_f_sin_set(angle_step * 0);
//            OVAR_Back_init(traget, 30, 5);
//            ovar_para.state = back;
//            return 0;
//        }
//    }
//    if (ovar_para.state == back)
//    {
//        ovar_para.back.motor.ms_esccape++;
//        res = motor_speed_cal(0, &speeds, &ovar_para.back);
//        if (res == 0)
//        {
//            tim_f_sin_set(0);
//            return 1;
//        }
//        tim_f_sin_set(angle_step * speeds);
//    }
//    return 0;
}
static uint8_t OVAR_handler(void)
{
    uint8_t res = 0;
    res = motor_set();
    if (res)
    {
        ovar_para.state = end;
        return 1; // end turn off tim4
    }
    return 0;
}

uint8_t OVAR_Machine_Init(float vel, uint32_t MillSec)
{

#if 0
    Slave1_Set_Machine_Cb(CONT_handler);
    ovar_para.Tick = 0;
    ovar_para.state = running;
    ovar_para.MillSecReq = 2000;
#else
    if (vel > 360 || vel < -360)
        return 0;
    ovar_para.vel = vel;
    ovar_para.MillSecReq = MillSec;
    ovar_para.Tick = 0;
    ovar_para.state = running;
		  Slave1_Step_Generator_Init(50000, 10);
    Slave1_Set_Machine_Cb(OVAR_handler);
#endif
    return 1;
}

uint8_t OVAR_Machine_Stop(void)
{
    ovar_para.CurrentMillSec = 0;
    ovar_para.state = back;
    return 0;
}


#define abs(n) ((n > 0) ? (n) : (-(n)))
void OVAR_Back_init(float tragetPos, float sps, float accMs)
{
    float speed, pos = Slave1_Get_Encode_Angle();
    ovar_para.back.motor.ms_esccape = 0;
    ovar_para.back.acc_time = accMs;
    ovar_para.back.sps = sps;
    if (tragetPos > pos)
        ovar_para.back.dir = 0;
    else
        ovar_para.back.dir = 1;

    ovar_para.back.lenght = abs(tragetPos - pos);
    motor_speed_cal(1, &speed, &ovar_para.back);
}

uint8_t OVAR_Machine_Get_Count(uint32_t *MillSecReq, uint32_t *CurrentMillSec)
{
    if (ovar_para.state == end)
        return 0;
    if (MillSecReq != 0)
        *MillSecReq = ovar_para.MillSecReq;
    if (CurrentMillSec != 0)
        *CurrentMillSec = ovar_para.CurrentMillSec;
    return 1;
}
