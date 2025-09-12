/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-24 14:44:19
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2024-09-06 09:54:46
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\implement\Slave_Vor_Ctrl.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stm32f4xx.h"
#include "Slave_Inc_Ctrl.h"
#include "math.h"
#include "delay.h"
#include "../HARDWARE/SLAVE/Slave1_IO.h"
#include "../HARDWARE/SLAVE/Slave1_Timer.h"
#include "../HARDWARE/INC/INC_CTRl.h"
extern void Rk3588_Printf(const char *strOutputString, ...);
typedef struct
{
    enum
    {
        end,
        running,
        retract,
        error,
        manual
    } state;
    int8_t manual_dir;
    uint32_t Tick;
    uint32_t MaxCount;
    float angleReq;
    // float current_angle;
} Inc_Machine_parameter;
Inc_Machine_parameter inc_para;

extern uint8_t JY60_Get_Inc(float *angle);
extern float JY60_Get_IncAsync();

uint8_t Inc_ctrl(void)
{
    const int16_t angle_nomovecounterMax = 10000;
    static uint8_t Angle_nonreqTime;
    static int16_t last_angle, angle_nomovecounter;
    static float angle = 0;
    angle = JY60_Get_IncAsync();
    uint8_t exit_flag = 0;
    // if (JY60_Get_IncAsync(&angle) == 0)
    // {
    //     //        Angle_nonreqTime++;
    //     //        if (Angle_nonreqTime == 255)
    //     //        {
    //     //            inc_para.state = error;
    //     //            return 1;
    //     //        }
    // }
    // else
    //     Angle_nonreqTime = 0;
    if (inc_para.state == running)
    {
        int16_t bias = ((inc_para.angleReq - angle) * 10.0f);
        //   Rk3588_Printf("\r\n bias %d",bias);
        if (last_angle == bias)
        {
            angle_nomovecounter++;
        }
        else
            angle_nomovecounter = 0;
        if (angle_nomovecounter > angle_nomovecounterMax)
        {
            exit_flag = 1;
            // inc_para.state = end;
        }
        last_angle = bias;
        //				if(angle_nomovetime
        Angle_nonreqTime = bias;
        if (bias > 0)
        {
            INC_IO_Set(1);
        }
        else if (bias < 0)
        {
            INC_IO_Set(2);
        }
        else
        {
            INC_IO_Set(0);
            if (inc_para.angleReq < 0.5)
            {
                inc_para.state = retract;
            }
            else
            {
                exit_flag = 1;
                // inc_para.state = end;
            }
        }
    }
    if (inc_para.state == retract)
    {
        inc_para.Tick++;
        INC_IO_Set(2);
        if (inc_para.Tick > 3000)
        {
            exit_flag = 1;
            // inc_para.state = end;
            // INC_IO_Set(0);
        }
    }
    if (inc_para.state == manual)
    {
        uint8_t io_cmd = (inc_para.manual_dir == 1) ? 1 : (inc_para.manual_dir == 0) ? 0
                                                      : (inc_para.manual_dir == -1)  ? 2
                                                                                     : 0;
        inc_para.Tick++;
        INC_IO_Set(io_cmd);
        if (inc_para.Tick > 200)
        {
            exit_flag = 1;
            // inc_para.state = end;
        }
    }
    if (exit_flag == 1)
    {
        return 1;
    }
    return 0;
}

uint8_t INC_handler(void)
{
    uint8_t res = 0;
    res = Inc_ctrl();
    if (res)
    {
        INC_IO_Set(0);
				inc_para.state = end;
        return 1; // end turn off tim4
    }
    return 0;
}

uint8_t INC_Machine_Init(float angle, uint32_t Maxcount)
{
#ifndef HARDWARE_TEST
#else
#endif
    INC_IO_INIT();
    Slave1_Set_Machine_Cb(INC_handler);
    inc_para.state = running;
    inc_para.angleReq = angle;
    return 1;
}

uint8_t INC_Machine_Manual_Ctrl(int8_t direction)
{
#ifndef HARDWARE_TEST
#else
#endif
    if (inc_para.state == end)
    {
        inc_para.state = manual;
        inc_para.Tick = 0;
        INC_IO_INIT();
        Slave1_Set_Machine_Cb(INC_handler);
    }
    inc_para.Tick = 0;
    inc_para.manual_dir = direction;
    return 1;
}

uint8_t INC_Machine_Stop(void)
{
    inc_para.state = end;
    return 1;
}

uint8_t INC_Machine_Get_Count(float *angleReq, float *CurrentAngle)
{
    if (inc_para.state == end)
        return 0;
    if (angleReq != 0)
        *angleReq = inc_para.angleReq;
    if (CurrentAngle != 0)
        JY60_Get_Inc(CurrentAngle);
    return 1;
}
