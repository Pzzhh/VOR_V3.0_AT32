/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-24 10:26:50
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2024-09-09 10:44:47
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\INTERFACE\UI\control\task_control.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "task_control.h"
#include "../other/system_function.h"
#include "../Task/task_info_struct.h"
#include "control_Hardware_API.h"
#include "stdint.h"

#ifndef STM32F40_41xxx
#include "Windows.h"
#include <stdio.h>

#else
#include "FreeRTOS.h"
#include "task.h"
#endif

#include "control_VOR.h"
#include "control_CONT.h"
#include "control_OVAR.h"
#include <stdarg.h>

Task_Parameter_Struct TaskArray[10];
typedef struct
{
	uint16_t currentTaskIndex;
	uint32_t remainingTime;
} Task_Control_Status;
// static char msg_buffer[50];

static Task_control_info* control;
extern uint8_t Ctrl_Msg_Printf(const char* format,
	...);
extern void Rk3588_Printf(const char* strOutputString, ...);
extern void Rk3588_L_Printf(const char* strOutputString, ...);

void RK3588_SendTaskInfo(uint16_t tasksize)
{
	Task_Parameter_Struct* e;
	char* mousename = (char*)malloc(50);
	mousename[0] = 0;
	HAL_Get_UI_MouseName(mousename, 50);
	if (mousename[0] == 0)
		strncpy(mousename, "unnamed", 50);
	Rk3588_Printf("&%d", tasksize);
	Rk3588_L_Printf("&%d", tasksize);
	for (int i = 0; i < tasksize; i++)
	{
		e = &TaskArray[i];
		switch (e->mode)
		{
		case Task_VOR:
			Rk3588_Printf("&%s_VOR_T%dV%dF%02d", mousename, e->VOR.Counter, (int)e->VOR.Vel, (int)(e->VOR.Freq * 10.0f));
			Rk3588_L_Printf("&%s_VOR_T%dV%dF%02d", mousename, e->VOR.Counter, (int)e->VOR.Vel, (int)(e->VOR.Freq * 10.0f));
			break;
		case Task_Continue:
			Rk3588_Printf("&%s_CONT_S%dV%d", mousename, e->CONT.Sec, (int)e->CONT.Vel);
			Rk3588_L_Printf("&%s_CONT_S%dV%d", mousename, e->CONT.Sec, (int)e->CONT.Vel);
			break;
		case Task_OVAR:
			Rk3588_Printf("&%s_OVAR_S%dV%dI%02d", mousename, e->OVAR.Sec, (int)e->OVAR.Vel, (int)(e->OVAR.Inc_Degree));
			Rk3588_L_Printf("&%s_OVAR_S%dV%dI%02d", mousename, e->OVAR.Sec, (int)e->OVAR.Vel, (int)(e->OVAR.Inc_Degree));
			break;
		case Task_VHIT:
			Rk3588_Printf("&%s_VHIT_T%d", mousename, e->VHIT.Counter);
			Rk3588_L_Printf("&%s_VHIT_T%d", mousename, e->VHIT.Counter);
			break;
		case Task_TC:
			Rk3588_Printf("&%s_TC_S%dV%d", mousename, e->TC.Sec, e->TC.Vel);
			Rk3588_L_Printf("&%s_TC_S%dV%d", mousename, e->TC.Sec, e->TC.Vel);
			break;

		default:
			break;
		}
	}
	Rk3588_Printf("@");
	Rk3588_L_Printf("@");
	free(mousename);
}

void RK3588_End_Printf(void)
{
	Rk3588_Printf("finish");
}

void RK3588_terminal_Printf(void)
{
	Rk3588_Printf("terminal");
}

void RK3588_Initial_Printf(void)
{
	Rk3588_Printf("clear");
	Rk3588_L_Printf("clear");
}

void RK3588_interrupt_Printf(void)
{
	Rk3588_Printf("interrupt");
	Rk3588_L_Printf("interrupt");
}

void task_interval_handle(int index)
{
	const uint32_t waitMillSec = 5000;
	uint32_t time = ControlGetTick(), remainingTime = 0;
	// uint16_t len = sprintf(msg_buffer, "Interval: ID:%d", index);
	// Message_Center_Send("PAGE1", 0, msg_buffer, len);
	/*  Message_Center_Send_prinft(
		  "PAGE1", 0, 0,
		  "Interval: ID:%d", index);*/
		  /*HAL_Set_UI_Page1_Msg("Interval: ID:%d", index);*/
	while (remainingTime < waitMillSec)
	{
		remainingTime = ControlGetTick() - time;
		Ctrl_Msg_Printf("interval %ds", remainingTime / 1000);
		if (control->State_Bit.Exit) // for exit
		{
			return;
		}
		ControlDelay(100);
	}
}

uint8_t Task_control_Begin(Task_control_info* e)
{
	if (e->State_Bit.IsRunning == 1)
	{
		return 0;
	}
	e->taskCount = HAL_Task_GetArray(TaskArray, 10);
	if (e->taskCount == 0 || e->taskCount > 20)
		return 0;
	e->taskArray = TaskArray;
	e->State_Bit.flag = 0;
	e->State_Bit.Init = 1;
	return 1;
}

uint8_t Task_control_ReqStop(Task_control_info* e)
{
	if (e->State_Bit.IsRunning == 0)
	{
		return 0;
	}
	e->State_Bit.Exit = 1;
	return 1;
}

void Task_control_handler(Task_control_info* e)
{
	uint8_t Rk3588_ReadyFlag_1 = 0, Rk3588_ReadyFlag_2 = 0;
	if (e == 0)
		return;
	control = e;
	e->State_Bit.Init = 1;
	e->State_Bit.Init = 0;
	e->State_Bit.ExInit = 0;
	Ctrl_Msg_Printf(" ");
	// uint16_t len = 0;
BEGIN_POS:
	// get all task need to implement
	while (1)
	{
		if (e->State_Bit.Init == 1)
		{
			e->State_Bit.flag = 0;
			e->Error_Bit.flag = 0;
			if (e == 0 || e->taskArray == 0)
			{
				e->Error_Bit.paraError = 1;
				continue;
			}
			break;
		}
		SaftExitDelay(100, 0);
	}

	e->State_Bit.IsRunning = 1;

	uint16_t task_size = e->taskCount;
	RK3588_SendTaskInfo(task_size);

	for (int i = 0; i < task_size; i++)
	{
		Task_Parameter_Struct* task = &e->taskArray[i];
		e->currentCount = i;
		e->currentparcent = 0;
		e->UI_para.state = Interval;
		task_interval_handle(i);
		if (e->State_Bit.Exit)
		{
			RK3588_terminal_Printf();
			break;
		}

		uint8_t isretract = 1;
		switch (task->mode)
		{
		case Task_VOR:
			VorControlFunction(task, e);
			break;
		case Task_Continue:
			ContControlFunction(task, e);
			break;
		case Task_OVAR:
			isretract = 1;
			if (i < (task_size - 1) && e->taskArray[i + 1].mode == Task_OVAR)
				isretract = 0;
			OvarControlFunction(task, e, isretract);
			break;
		default:
			break;
		}
	}
	RK3588_End_Printf();
	Ctrl_Msg_Printf(" ");
	// len = sprintf(msg_buffer, "End");
	// Message_Center_Send("PAGE1", 0, msg_buffer, len);
	/*HAL_Set_UI_Page1_Msg("End");*/
	e->State_Bit.flag = 0;
	e->Error_Bit.flag = 0;
	goto BEGIN_POS;
}
