#include "control_OVAR.h"
#include "control_Hardware_API.h"
#include "../other/system_function.h"
#include "control.h"
#include "stdint.h"
#define use_windows
#ifndef STM32F40_41xxx
#include "Windows.h"
#include <stdio.h>
#else
#include "../implement/Slave_OVAR_Ctrl.h"
#endif // use_windows
// 无UI控制
// 返回message

struct
{
	uint32_t time;
	uint32_t SetSec;
	float angle;
	float Vel;
} ovar_info;

uint8_t hal_Slave_OVAR_Init(Task_Parameter_Struct* e)
{
#ifndef STM32F40_41xxx
	ovar_info.time = ControlGetTick();
	ovar_info.SetSec = e->CONT.Sec;
#else

	ovar_info.Vel = e->OVAR.Vel;
	ovar_info.SetSec = e->OVAR.Sec;
	ovar_info.angle = e->OVAR.Inc_Degree;
	if (e->mode == Task_OVAR)
		OVAR_Machine_Init(e->OVAR.Vel, e->OVAR.Sec * 1000);
#endif // !STM32F40_41xxx
	return 1;
}

uint8_t hal_Slave_OVAR_Get_State(uint32_t* remainingSec, uint32_t* parcent)
{
#ifndef STM32F40_41xxx
	uint32_t currentSec = (ControlGetTick() - ovar_info.time) / 1000.0f;
	if (parcent != 0)
		*parcent = currentSec * 100 / ovar_info.SetSec;
	if (currentSec < ovar_info.SetSec)
	{
		*remainingSec = ovar_info.SetSec - currentSec;
		return 1;
	}
	*remainingSec = 0;
	return 0;
#else
	uint32_t counterReq = 0, CurrentCount = 0;
	uint8_t res = OVAR_Machine_Get_Count(&counterReq, &CurrentCount);
	if (parcent != 0)
		*parcent = CurrentCount * 100 / counterReq;
	*remainingSec = counterReq - CurrentCount;
	return res;
#endif // !STM32F40_41xxx
}

uint8_t hal_Slave_OVAR_Stop(void)
{
#ifndef STM32F40_41xxx
	uint32_t currentSec = (ControlGetTick() - ovar_info.time) / 1000.0f;
	ovar_info.SetSec = currentSec + 1;
#else
	OVAR_Machine_Stop();
#endif // !STM32F40_41xxx
	return 1;
}

void Inc_handle(float angle)
{
	HAL_Incline_Init(angle, 0);
	uint8_t inc_flag = 1;

	while (inc_flag)
	{
		float current_angle;
		inc_flag = HAL_Incline_Get_State(&current_angle);
		Ctrl_Msg_Printf("inc angle %d", (int)current_angle);
		MYPRINTF("%.3f", current_angle);
		SaftExitDelay(10, 0);
		MYPRINTF("\r");
	}
}

static void motor_handle(Task_Parameter_Struct* task, Task_control_info* e)
{
	const int camWaitTime_s = 5;
	uint8_t OVAR_flag = 1;
	int32_t LastCount = -1;

	uint8_t CAM_State = HAL_CAM_REC_Set(1);
	for (int i = 0; i < camWaitTime_s; i++)
	{
		(CAM_State == 0 && i < 2) ?
			Ctrl_Msg_Printf("camera error") :
			Ctrl_Msg_Printf("start after %ds", i);
		SaftExitDelay(1000, 0);
	}
	hal_Slave_OVAR_Init(task);
	MYPRINTF("\r\n");
	uint32_t count,parcent;
	while (OVAR_flag)
	{
		OVAR_flag = hal_Slave_OVAR_Get_State(&count,&parcent);
		e->currentparcent = parcent;
		if (LastCount != count)
		{
			Ctrl_Msg_Printf("%d OVAR Done:%d%%", e->currentCount, (int)parcent);
			LastCount = count;
		}
		if (e->State_Bit.Exit) // for exit
		{
			Ctrl_Msg_Printf("%d:OVAR Terminated", e->currentCount);
			hal_Slave_OVAR_Stop();
		}
		MYPRINTF("%3d", count);
		// wait motor infinsh
		SaftExitDelay(10, 0);
		MYPRINTF("\r");
	}
	Ctrl_Msg_Printf("%d:OVAR Done:100%%", e->currentCount);
	/*one sec for cam stop*/
	SaftExitDelay(1000, 0);
	CAM_State = HAL_CAM_REC_Set(1);
	if (CAM_State == 0)
	{
		Ctrl_Msg_Printf("CAM ERROR");
	}
	SaftExitDelay(1000, 0);
	/*--one sec for cam stop*/
}

uint8_t OvarControlFunction(Task_Parameter_Struct* task,
	Task_control_info* e,
	uint8_t isretract)
{
	MYPRINTF("\r\n ovar begin");
	MYPRINTF("\r\n");
	// 执行部分
	MYPRINTF("\r\n");
	e->UI_para.state = taskruning;
	Inc_handle(task->OVAR.Inc_Degree);
	motor_handle(task, e);
	if (isretract == 1)
		Inc_handle(0);
	Ctrl_Msg_Printf("OVAR end ");
	MYPRINTF("\r\n ovar end");
	e->UI_para.state = end;
	return 0;
}