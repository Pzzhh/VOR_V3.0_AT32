/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-24 14:44:19
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2024-09-21 14:42:40
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\implement\Slave_Vor_Ctrl.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stm32f4xx.h"
#include "Slave_Vor_Ctrl.h"
#include "math.h"
#include "delay.h"
#include "../HARDWARE/SLAVE/Slave1_IO.h"
#include "../HARDWARE/SLAVE/Slave1_Timer.h"

int sin_time = 0;
int tim_count, last_count, plus_f = 1;
extern void  Motor_Spd_Pid(float speed);
typedef struct
{
    enum
    {
        running,
        end,
    } state;
    uint32_t Tick;
    float freq;
    float vel;
    // Task_Parameter_Struct info;
    uint32_t CurrentCounter;
    uint32_t counterReq;
} Vor_Machine_parameter;
Vor_Machine_parameter vor_para;
#define Pi 3.1415926
float C610Spd;
extern uint8_t HAL_CAM_SET_sign_led(void);
uint8_t Slave_motor(void)
{
    if (vor_para.state == running)
    {
        float time_max = 1000;
        float sin_data;
				uint32_t PhaseZero = 1000.0f/vor_para.freq;
			if(vor_para.Tick%PhaseZero==0)
					HAL_CAM_SET_sign_led();
        int tem = time_max / vor_para.freq;
        sin_data = sin(((float)2.0f * Pi * vor_para.freq * vor_para.Tick / 1000.0f));
        vor_para.CurrentCounter = vor_para.freq * vor_para.Tick / 1000.0f;
			 vor_para.Tick++;
        Motor_Spd_Pid( -vor_para.vel*sin_data*33.33);
        tim_f_sin_set(angle_step * sin_data * vor_para.vel);
        if (vor_para.CurrentCounter >= vor_para.counterReq)
            return 1;
    }
    return 0;
}

uint8_t VOR_handler(void)
{
    uint8_t res = 0;
    //    switch (vor_para.state)
    //    {
    //    case running:
    res = Slave_motor();
    /* code */
    //        break;
    //    case end:
    //        /* code */
    //        break;
    //    default:
    //        break;
    //    }
    if (res)
    {       
			Motor_Spd_Pid(0);
        vor_para.state = end;

        return 1; // end turn off tim4
    }
    return 0;
}

uint8_t VOR_Machine_Init(float freq, float vel, uint32_t count)
{
#ifdef HARDWARE_TEST
    Slave1_Set_Machine_Cb(VOR_handler);
    vor_para.Tick = 0;
    vor_para.state = running;
    vor_para.counterReq = 2;
#else
    if (vel > 360 ||
        freq <= 0 || freq >= 15)
        return 0;
    vor_para.freq = freq;
    vor_para.vel = vel;
    vor_para.counterReq = count;
    Slave1_Step_Generator_Init(50000, 10);
    Slave1_Set_Machine_Cb(VOR_handler);
    vor_para.Tick = 0;
    vor_para.state = running;
#endif
    return 1;
}

uint8_t VOR_Machine_Stop(void)
{
    vor_para.counterReq = vor_para.CurrentCounter + 1;
    return 1;
}

uint8_t VOR_Machine_Get_Count(uint32_t *counterReq, uint32_t *CurrentCounter)
{
    if (vor_para.state == end)
        return 0;
    if (counterReq != 0)
        *counterReq = vor_para.counterReq;
    if (CurrentCounter != 0)
        *CurrentCounter = vor_para.CurrentCounter;
    return 1;
}
