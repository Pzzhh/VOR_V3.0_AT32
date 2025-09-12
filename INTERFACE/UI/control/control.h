/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-22 16:00:07
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @LastEditTime: 2024-08-12 17:41:30
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\INTERFACE\UI\control\control.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __control_h_
#define __control_h_
#ifdef __cplusplus
extern "C"
{
#endif //
#include "stdint.h"
#include "../Task/task_info_struct.h"
#ifndef STM32F40_41xxx
#include "Windows.h"
#include <stdio.h>
#endif // use_windows

#define control_cb_array_size 10
#define CmdCheck(in, ref) strncmp(in, ref, sizeof(ref) - 1)

	typedef enum
	{
		Interval,
		ready,
		taskruning,
		runing,
		end,
	} task_runing;
	//typedef enum
	//{
	//	CMD_add,
	//	CMD_Move,
	//	CMD_Del,
	//	CMD_Set,
	//} CMD_Enum;

	typedef struct
	{
		union
		{
			uint8_t flag;
			struct
			{
				uint8_t Init : 1;
				uint8_t ExInit : 1;
				uint8_t IsRunning : 1;
				uint8_t Exit : 1;
				uint8_t finish : 1;
			};
		} State_Bit;
		union
		{
			uint8_t flag;
			struct
			{
				uint8_t paraError : 1;
			};
		} Error_Bit;
#ifndef STM32F40_41xxx
		HANDLE hThread;
#endif // use_windows
		struct
		{
			uint32_t sec;
			task_runing state;
		} UI_para;
		uint8_t ismessageUpdata;
		uint8_t message[50];
		Task_Parameter_Struct* taskArray;
		uint16_t taskCount;
		uint16_t currentCount;
		uint16_t currentparcent;
		uint16_t taskRemainSec;
	} Task_control_info;
	void controlInit(void);
	// 0 代表无返回信息
	typedef uint8_t(*UI_function)(uint8_t* cmd, uint8_t* meassage, Task_control_info* e);
	typedef struct
	{
		UI_function cb;
		uint8_t message[50];
	} UI_Function_struct;
	uint8_t Control_Set_cb(UI_Function_struct* function_array,
		uint16_t index,
		UI_function cb);
	uint8_t Control_Remove_cb(UI_Function_struct* function_array,
		uint16_t index,
		UI_function cb);

	uint8_t Ctrl_Msg_Printf(const char* format,
		...);

	extern UI_Function_struct control_cb_array[control_cb_array_size];
#ifdef __cplusplus
}
#endif //

#endif // ! UI_Paremeter_Set_H_
