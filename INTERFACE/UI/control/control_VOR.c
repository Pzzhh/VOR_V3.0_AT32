/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-22 16:00:07
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @LastEditTime: 2024-08-08 11:11:54
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\INTERFACE\UI\control\control_VOR.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "control_VOR.h"
#include "control_Hardware_API.h"
#include "../other/system_function.h"
#include "control.h"
#include "stdint.h"
#define use_windows
#ifndef STM32F40_41xxx
#include "Windows.h"
#include <stdio.h>
#else
#include "../implement/Slave_Vor_Ctrl.h"
#endif // use_windows
 // 无UI控制
// 返回message

struct
{
	uint32_t time;
	uint32_t SetCount;
	float Vel;
	float Freq;
} vor_info;

uint8_t HAL_Slave_VOR_Init(Task_Parameter_Struct* e)
{
#ifndef STM32F40_41xxx
	vor_info.time = ControlGetTick();
	vor_info.Freq = e->VOR.Freq;
	vor_info.SetCount = e->VOR.Counter;
#else
	if (e->mode == Task_VOR)
		VOR_Machine_Init(e->VOR.Freq, e->VOR.Vel, e->VOR.Counter);
#endif // !STM32F40_41xxx

	return 1;
}

uint8_t HAL_Slave_VOR_Stop(void)
{
#ifndef STM32F40_41xxx
	uint32_t currentCount = vor_info.Freq * (ControlGetTick() - vor_info.time) / 1000.0f;
	vor_info.SetCount = currentCount + 1;
#else
	VOR_Machine_Stop();
#endif // !STM32F40_41xxx
	return 1;
}

uint8_t HAL_Slave_VOR_Get_State(uint32_t* remainingCount,uint32_t * parcent)
{
#ifndef STM32F40_41xxx
	uint32_t currentCount = vor_info.Freq * (ControlGetTick() - vor_info.time) / 1000.0f;
	if (parcent != 0)
		*parcent = currentCount * 100 / vor_info.SetCount;
	if (currentCount < vor_info.SetCount)
	{
		*remainingCount = vor_info.SetCount - currentCount;
		return 1;
	}
	*remainingCount = 0;
	return 0;
#else
	uint32_t counterReq = 0, CurrentCount = 0;
	uint8_t res = VOR_Machine_Get_Count(&counterReq, &CurrentCount);
	if (parcent != 0&& counterReq!=0)
		*parcent = CurrentCount * 100 / counterReq;
	*remainingCount = counterReq - CurrentCount;
	return res;
#endif // !STM32F40_41xxx
}

uint8_t VorControlFunction(Task_Parameter_Struct* task, Task_control_info* e)
{
	const int camWaitTime_s = 5;
	e->UI_para.state = ready;
	MYPRINTF("\r\n vor begin");
	MYPRINTF("\r\n");
	// 执行部分
	MYPRINTF("\r\n");
	uint8_t CAM_State = HAL_CAM_REC_Set(1);
	for (int i = 0; i < camWaitTime_s; i++)
	{
		(CAM_State == 0 && i < 2) ?
			Ctrl_Msg_Printf("camera error") :
			Ctrl_Msg_Printf("start after %ds", i);
		SaftExitDelay(1000, 0);
	}
	/*motor set running configure*/
	HAL_Slave_VOR_Init(task);
	/*notice control thread current state*/
	e->UI_para.state = taskruning;
	/*get vor machine flag*/
	uint8_t VOR_machine_flag = 1;
	int32_t LastCount = -1;
	uint32_t count,parcent;
	/*waiting vor machine finish*/
	while (VOR_machine_flag)
	{
		VOR_machine_flag = HAL_Slave_VOR_Get_State(&count, &parcent);
		e->currentparcent = parcent; 
		if (LastCount != count)
		{
			Ctrl_Msg_Printf("%d:VOR Done:%d%%", e->currentCount, parcent);
			LastCount = count;
		}
		if (e->State_Bit.Exit) // for exit
		{
			Ctrl_Msg_Printf("%d:VOR Terminated", e->currentCount);
			HAL_Slave_VOR_Stop();
		}
		MYPRINTF("%3d", count);
		// wait motor infinsh
		SaftExitDelay(50, 0);
		
		MYPRINTF("\r");
	}
	Ctrl_Msg_Printf("%d:VOR Done:100%%", e->currentCount);
	/*one sec for cam stop*/
	SaftExitDelay(1000, 0);
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
