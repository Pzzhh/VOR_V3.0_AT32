/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-22 16:00:07
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2025-02-06 16:47:30
 * @FilePath: \USERd:\workfile\é¡¹ç›®3 vor\software\TC_V2.0\INTERFACE\UI\control\control_TC.c
 * @Description: è¿™æ˜¯é»˜è?¤è?¾ç½®,è¯·è?¾ç½®`customMade`, æ‰“å¼€koroFileHeaderæŸ¥çœ‹é…ç½® è¿›è?Œè?¾ç½®: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "control_TC.h"
#include "control_Hardware_API.h"
#include "../other/system_function.h"
#include "control.h"
#include "stdint.h"
#define use_windows
#ifndef STM32F40_41xxx
#include "Windows.h"
#include <stdio.h>
#else
#include "../implement/Slave_TC_Ctrl.h"
#endif // use_windows
 // æ— UIæ§åˆ¶
// è¿”å›message
const int singalCount = 1 * 1000;
struct
{
	uint32_t time;
	uint32_t RemainTime;
	uint8_t flag_pause;
	uint32_t SetCount;
	float Vel;
} tc_info;

uint8_t HAL_Slave_TC_Init(Task_Parameter_Struct* e)
{
#ifndef STM32F40_41xxx
	tc_info.time = ControlGetTick();
	tc_info.SetCount = e->TC.Counter;
	tc_info.flag_pause = 0;
#else
	if (e->mode == Task_TC)
		TC_Machine_Init(e->TC.Counter,e->TC.Vel);
#endif // !STM32F40_41xxx

	return 1;
}

uint8_t HAL_Slave_TC_Stop(void)
{
#ifndef STM32F40_41xxx
	uint32_t currentCount = (ControlGetTick() - tc_info.time) / singalCount;
	tc_info.SetCount = currentCount + 1;
#else
	TC_Machine_Stop();
#endif // !STM32F40_41xxx
	return 1;
}

uint8_t HAL_Slave_TC_Pause(uint8_t enable)
{
#ifndef STM32F40_41xxx
	if (enable)
	{
		tc_info.flag_pause = 1;
		tc_info.RemainTime = ControlGetTick() - tc_info.time;
	}
	else
	{
		tc_info.time = ControlGetTick() - tc_info.RemainTime;
		tc_info.flag_pause = 0;
	}
#else
	TC_Machine_Pause();
#endif // !STM32F40_41xxx
	return 1;
}

uint8_t HAL_Slave_TC_Get_State(uint32_t* remainingCount, uint32_t* parcent)
{
#ifndef STM32F40_41xxx
	if (tc_info.flag_pause)
		return Imp_paused;
	uint32_t currentCount = (ControlGetTick() - tc_info.time) / singalCount;
	if (parcent != 0)
		*parcent = currentCount * 100 / tc_info.SetCount;
	if (currentCount < tc_info.SetCount)
	{
		*remainingCount = tc_info.SetCount - currentCount;
		return Imp_running;
	}
	*remainingCount = 0;
	return Imp_finsih;
#else
	uint32_t counterReq = 0, CurrentCount = 0;
	uint8_t res = TC_Machine_Get_Count(&counterReq, &CurrentCount);
	if (parcent != 0 && counterReq != 0)
		*parcent = CurrentCount * 100 / counterReq;
	*remainingCount = counterReq - CurrentCount;
	return res;
#endif // !STM32F40_41xxx
}



uint8_t TcControlFunction(Task_Parameter_Struct* task, Task_control_info* e)
{
	const int camWaitTime_s = 5;
	e->UI_para.state = ready;
	MYPRINTF("\r\n vor begin");
	MYPRINTF("\r\n");
	// æ‰§è?Œéƒ¨åˆ?
	MYPRINTF("\r\n");
Pause_Resume:
	uint8_t CAM_State = HAL_CAM_REC_Set(1);
	for (int i = 0; i < camWaitTime_s; i++)
	{
		(CAM_State == 0 && i < 2) ? Ctrl_Msg_Printf("camera error") : Ctrl_Msg_Printf("Run after %ds", camWaitTime_s - i);
		SaftExitDelay(1000, 0);
	}
	/*motor set running configure*/
	HAL_Slave_TC_Init(task);
	/*notice control thread current state*/
	e->UI_para.state = taskruning;
	/*get vor machine flag*/
	uint8_t TC_machine_flag = 1, CamIsStop = 0;
	int32_t LastCount = -1;
	uint32_t count, parcent;
	e->State_Bit.reqPause = 0;
	e->State_Bit.VHIT_Next = 0;
	/*waiting vor machine finish*/
	while (TC_machine_flag)
	{
		TC_machine_flag = HAL_Slave_TC_Get_State(&count, &parcent);
		if (LastCount != count && TC_machine_flag == Imp_running)
		{
			Ctrl_Msg_Printf("%d:%s Done:%d%%", e->currentCount+1,"TC", parcent);
			LastCount = count;
		}
		if (e->State_Bit.Exit) // for exit
		{
			Ctrl_Msg_Printf("%d:%s #A52A2A Terminated#", e->currentCount+1,"TC");
			HAL_Slave_TC_Stop();
		}
		if (e->State_Bit.reqPause == 1)
		{
			if (TC_machine_flag == Imp_running)
			{
				HAL_Slave_TC_Pause(1);
				Ctrl_Msg_Printf("%d:%s Pause", e->currentCount+1,"TC");
				Rk3588_Send_Pause;
				e->State_Bit.isPause = 1;
				// if (CamIsStop == 0)
				// 	HAL_CAM_REC_Set(1);
				// CamIsStop = 1;
				HAL_CAM_REC_Set(1);
			}
			else if (TC_machine_flag == Imp_paused)
			{
				e->State_Bit.isPause = 0;
				goto Pause_Resume;
				//Ctrl_Msg_Printf("%d:%s Done:%d%%", e->currentCount,"TC", parcent);
				//HAL_Slave_TC_Pause(0);
				//HAL_CAM_REC_Set(1);
			}
			e->State_Bit.reqPause = 0;
		}
		MYPRINTF("%3d", count);
		// wait motor infinsh
		SaftExitDelay(50, 0);

		MYPRINTF("\r");
	}
	if (e->State_Bit.Exit == 0)
		Ctrl_Msg_Printf("%d:%s Done:100%%", e->currentCount+1,"TC");
	/*one sec for cam stop*/
	SaftExitDelay(1000, 0);
	if (CamIsStop == 0)
		CAM_State = HAL_CAM_REC_Set(1);
	if (CAM_State == 0)
	{
		Ctrl_Msg_Printf("CAM ERROR");
	}
	/*--one sec for cam stop*/
	MYPRINTF("\r\n vor end");
	e->UI_para.state = end;
	return 0;
}
