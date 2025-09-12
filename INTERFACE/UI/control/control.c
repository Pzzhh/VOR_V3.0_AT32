#include "control.h"
#include "control_Hardware_API.h"
#include "../../commuication/communication_API.h"
#include "task_control.h"
#include "../other/system_function.h"
#include "stdint.h"
#include "../other/meassage_center.h"
#include "stdarg.h"

#ifndef STM32F40_41xxx
#include "Windows.h"
#include <stdio.h>
#else
#include "FreeRTOS.h"
#include "task.h"
#endif // use_windows

#define use_windows
#ifdef STM32F40_41xxx
#else
typedef uint32_t TaskHandle_t;
#endif
void thread_create(void* function, Task_control_info* e, TaskHandle_t* control_thread, uint16_t sizeofstack);
TaskHandle_t task_ctrl_thread, contrl_thread;
Task_control_info control_info = { 0 };

#if 0
UI_Function_struct control_cb_array[control_cb_array_size];
uint8_t Control_Set_cb(UI_Function_struct* function_array,
	uint16_t index,
	UI_function cb)
{
	for (int i = 0; i < index; i++)
	{
		if (function_array[i].cb == 0)
		{
			function_array[i].cb = cb;
			return 1;
		}
	}
	return 0;
}

uint8_t Control_Remove_cb(UI_Function_struct* function_array,
	uint16_t index,
	UI_function cb)
{
	for (int i = 0; i < index; i++)
	{
		if (function_array[i].cb == cb)
		{
			function_array[i].cb = 0;
			return 1;
		}
	}
	return 0;
}

void control_run_cb_array(UI_Function_struct* function_array,
	uint16_t index,
	char cmd[50],
	char message[50],
	Task_Parameter_Struct* e)
{
	for (int i = 0; i < index; i++)
	{
		if (function_array[i].cb != 0)
		{
			uint8_t res = function_array[i].cb(message, cmd, 0);
			/*if (function_array[i].cb(message, cmd, e))*/
			/*return;*/
		}
	}
}
#endif

typedef enum
{
	StartCmdNone,
	StartCmdBegin,
	StartCmdStop
} Start_Cmd_Type;
Start_Cmd_Type Start_Stop_Flag = StartCmdNone;

uint8_t Ctrl_Get_Strat_Cmd()
{
	uint8_t cmd = Start_Stop_Flag;
	if (Start_Stop_Flag != StartCmdNone)
		Start_Stop_Flag = StartCmdNone;
	return cmd;
}

uint8_t Ctrl_Msg_Printf(const char* format,
	...)
{
	va_list args;
	va_start(args, format);
	uint16_t stringLen = vsprintf(control_info.message, format, args);
	control_info.ismessageUpdata = 1;
	va_end(args);
}

// return 0 ->ok
uint8_t Ctrl_Read_Ack(uint8_t* msg, uint16_t msg_size,
	uint8_t* src, uint16_t SrcSize)
{
	if (Msg_COMPARE("ReqReadState", msg))
	{
		sprintf(src, "ReadState %d %d",
			control_info.State_Bit.IsRunning, control_info.currentCount);
		return 0;
	}

	if (Msg_COMPARE("run_State?", msg))
	{
		uint8_t flag = control_info.State_Bit.IsRunning;

		sprintf(src, (flag) ? "isStart" : "isStop");
		return 0;
	}
	if (Msg_COMPARE("runStateDetail?", msg))
	{
		uint8_t flag = control_info.State_Bit.IsRunning;
		uint8_t stateIndex = control_info.UI_para.state;
		const char Statestr[][10] = { "interval","ready","run","run","end" };
		const char Runstr[][10] = { "isStart","isStop" };
		sprintf(src, "%s,%d,%s,%d", 
			Runstr[flag], control_info.currentCount, Statestr[stateIndex], control_info.currentparcent);	//1 当前情况 当前任务进度 当前任务总时间 
		return 0;
	}
	if (Msg_COMPARE("Msg?", msg))
	{
		if (control_info.ismessageUpdata)
		{
			strncpy(src, control_info.message, SrcSize);
			control_info.ismessageUpdata = 0;
			return 0;
		}
	}
	if (Msg_COMPARE("BLReadMsg", msg))
	{
		strncpy(src, control_info.message, SrcSize);
		return 0;
	}

	return 1;
}

uint8_t Ctrl_Write_Ack(uint8_t* msg, uint16_t msg_size,
	uint8_t* src, uint16_t SrcSize)
{
	if (Msg_COMPARE("ReqShift", msg))
	{
		if (control_info.State_Bit.IsRunning)
			Start_Stop_Flag = StartCmdStop;
		else
			Start_Stop_Flag = StartCmdBegin;
	}
	if (Msg_COMPARE("ReqStrat", msg))
	{
		Start_Stop_Flag = StartCmdBegin;
	}
	if (Msg_COMPARE("ReqStop", msg))
	{
		Start_Stop_Flag = StartCmdStop;
	}
	return 0;
}

uint8_t Maintain_Service_Read_ack(uint8_t* msg, uint16_t msg_size,
	uint8_t* src, uint16_t SrcSize)
{
	if (Msg_COMPARE("Camere LED", msg))
	{
		uint8_t res = HAL_CAM_SET_sign_led();
		return !res;
	}
	if (Msg_COMPARE("Camere wifi", msg))
	{
		uint8_t res = HAL_CAM_SET_Set();
		return !res;
	}
	if (Msg_COMPARE("Camere Rec", msg))
	{
		uint8_t res = HAL_CAM_REC_Set(0);
		return !res;
	}
	if (Msg_COMPARE("Camere Vol", msg))
	{
		int volX1000 = 0;
		uint8_t res = 1; // 0->success
		if (sscanf(msg, "Camere Vol %d", &volX1000) == 1)
		{
			if (volX1000 < 1800 && volX1000 > 500)
				res = HAL_CAM_SET_Led_Voltage(volX1000);
		}
		return !res;
	}
	if (Msg_COMPARE("Inc Up", msg))
	{
		HAL_Incline_Fouces_Move(1);
	}
	if (Msg_COMPARE("Inc down", msg))
	{
		HAL_Incline_Fouces_Move(-1);
	}
	if (Msg_COMPARE("FAN_pwm", msg))
	{
		int pwm = 0;
		if (sscanf(msg, "FAN_pwm %d", &pwm) == 1)
		{
			HAL_FAN_Set(pwm);
		}
		return 0;
	}

	return 1;
}

uint8_t ctrlWaitRk3588()
{
	uint8_t Startflag = Ctrl_Get_Strat_Cmd();
	if (Startflag != 0)
	{
		return 1;
	}
	return 0;
}

void Maintain_Service()
{
}

uint8_t Ctrl_Message_Center_init()
{
	/*Meassage_Center_Add("Ctrl");*/
	Message_Center_Add_Read_CB("Ctrl", Ctrl_Read_Ack);
	Message_Center_Add_Send_CB("Ctrl", Ctrl_Write_Ack);
	Message_Center_Add_Read_CB("Ctrl", Maintain_Service_Read_ack);
}

void controlfunction()
{
	extern void Task_mangager_Init();
	extern void Communication_Init();
	static Task_Parameter_Struct taskarray[15];
	int flag = 0;
	Start_Cmd_Type Startflag = StartCmdNone;
	uint8_t nowState = 0;
	char message[50], cmd[50] = { "" };

	thread_create(Task_control_handler, &control_info, &task_ctrl_thread, 1000);
	/*Meassage_Center_Add("page1");*/
	Ctrl_Message_Center_init();
	HAL_API_INIT();
	Task_mangager_Init();
	Communication_Init();
	while (1)
	{
		if (control_info.State_Bit.Init == 0)
		{
			if (Hal_Rk3588_Readarray(message) == 1) {
				if (Msg_COMPARE("wifi down", message))
				{
					HAL_CAM_SET_Set();

				}
			}
			Startflag = Ctrl_Get_Strat_Cmd();
			// Ctrl_Read_State_Ack(&control_info);
			if (Startflag == 0x01)
			{
				Task_control_Begin(&control_info);
			}
			if (Startflag == 0x02)
			{
				Task_control_ReqStop(&control_info);
			}
		}
		else
		{
			if (ctrlWaitRk3588() != 0)
			{
				control_info.State_Bit.ExInit = 1;
			}
		}
		ControlDelay(10);
		// e.ExitFlag = 1;
	}
}

void thread_create(void* function, Task_control_info* e, TaskHandle_t* control_thread, uint16_t sizeofstack)
{
#ifndef STM32F40_41xxx
	HANDLE hThread;
	DWORD dwThreadId;
	hThread = CreateThread(NULL,
		0,
		function,
		e,
		0,
		&dwThreadId);
#else
	//	static TaskHandle_t control_thread;
	volatile BaseType_t res =
		xTaskCreate((TaskFunction_t)function,
			(const char*)"Outside_motor",
			(uint16_t)sizeofstack,
			(void*)e,
			(UBaseType_t)2,
			(TaskHandle_t*)&control_thread);
#endif
}

void controlInit()
{
	Meassage_Center_Add("task");
	Meassage_Center_Add("page1");
	Meassage_Center_Add("Ctrl");
	thread_create(controlfunction, 0, &contrl_thread, 750);
	/*extern void SRV1_Encoder_Init();
	SRV1_Encoder_Init();*/
}
