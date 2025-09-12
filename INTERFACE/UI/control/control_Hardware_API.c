/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-25 14:38:08
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com
 * @LastEditTime: 2024-09-22 17:26:02
 * @FilePath: \USER  d:\workfile\项目3 vor\software\VOR_V2.0\INTERFACE\UI\control\control_Hardware_API.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "control_Hardware_API.h"
#include "../Task/task_stroage.h"
#include "../other/system_function.h"
#include "../Task/task_info_struct.h"
#include "../other/meassage_center.h"

#include "stdarg.h"
#include "stdio.h"

#ifdef STM32F40_41xxx
#include "../implement/Slave_Vor_Ctrl.h"
#include "../implement/Slave_Inc_Ctrl.h"
#include "../HARDWARE/CAN/can.h"
#include "../HARDWARE/FAN/FAN.h"
#endif // !STM32F40_41xxx

#define use_simluate 0
#if use_simluate

#endif //  use_simluate
struct
{
	uint32_t time;
	uint32_t SetCount;
	float Setangle;
	float Currentangle;
} inc_info;

uint8_t HAL_Incline_Init(float angle, uint32_t time)
{
	inc_info.Setangle = angle;
#ifdef STM32F40_41xxx
	INC_Machine_Init(angle, 0);

#endif
	return 1;
}
// 1 running
uint8_t HAL_Incline_Get_State(float *angle)
{
#if use_simluate
	uint32_t tick = ControlGetTick() - inc_info.time;
	if (inc_info.Currentangle >= inc_info.Setangle - 0.1 && inc_info.Currentangle <= inc_info.Setangle + 0.1)
		return 1;
	inc_info.Currentangle += (inc_info.Setangle - inc_info.Currentangle) * 0.05f;
	*angle = inc_info.Currentangle;
	return 0;
#endif
#ifdef STM32F40_41xxx
	return INC_Machine_Get_Count(0, angle);
#endif
	return 0;
}

uint8_t HAL_Incline_Fouces_Move(int8_t direction)
{
#ifdef STM32F40_41xxx
	return INC_Machine_Manual_Ctrl(direction);
#endif
	return 0;
}

uint8_t HAL_CAM_Init()
{
#ifdef STM32F40_41xxx
	//can_init(1, 6, 7, 6, 0); old 
	can_init(1, 10, 3, 7, 0);
		extern void C610_Ctrl_Init();
	C610_Ctrl_Init();
	// buf[0] 00  rec
	// buf[0] 01  set
	// buf[0] 02  U16_L U16_H  led voltage Set
	// buf[0] 04  cam Led Flush

#endif
	return 0;
}
// 0 发生错误 1 正常
uint8_t HAL_CAM_REC_Set(uint8_t flag)
{
	static uint8_t Cam_Flag = 0;
	uint8_t res = 1; // reture success
#ifdef STM32F40_41xxx
	//	if (Cam_Flag == flag)
	{
		const uint8_t CAN_CAM_MSG[8] = {
			0, 0, 0, 0, 0, 0, 0, 0};
		res = can_send_msg(0x00, (uint8_t *)CAN_CAM_MSG, 8); // can_send reture 1->fail 0->success
		res = (res == 1) ? 0 : 1;							 // exchange reture 1->success 0->fail
		Cam_Flag = flag;
	}
#endif
	return res;
}

// 0 发生错误 1 正常
uint8_t HAL_CAM_SET_Set(void)
{
	uint8_t res = 1; // reture success
#ifdef STM32F40_41xxx
	const uint8_t CAN_CAM_MSG[8] = {
		01, 0, 0, 0, 0, 0, 0, 0};
	res = can_send_msg(0x00, (uint8_t *)CAN_CAM_MSG, 8); // can_send reture 1->fail 0->success
	res = (res == 1) ? 0 : 1;							 // exchange reture 1->success 0->fail
#endif
	return res;
}

// 0 发生错误 1 正常
uint8_t HAL_CAM_SET_sign_led(void)
{
	uint8_t res = 1; // reture success
#ifdef STM32F40_41xxx
	const uint8_t CAN_CAM_MSG[8] = {
		04, 0, 0, 0, 0, 0, 0, 0};
	res = can_send_msg(0x00, (uint8_t *)CAN_CAM_MSG, 8); // can_send reture 1->fail 0->success
	res = (res == 1) ? 0 : 1;							 // exchange reture 1->success 0->fail
#endif
	return res;
}

// 0 error 1 normal
uint8_t HAL_CAM_SET_Led_Voltage(uint16_t Millivol)
{
	uint8_t res = 1; // reture success
	uint8_t *vol = (uint8_t *)&Millivol;
#ifdef STM32F40_41xxx
	const uint8_t CAN_CAM_MSG[8] = {
		02, vol[0], vol[1], 0, 0, 0, 0, 0};
	res = can_send_msg(0x00, (uint8_t *)CAN_CAM_MSG, 8); // can_send reture 1->fail 0->success
	res = (res == 1) ? 0 : 1;							 // exchange reture 1->success 0->fail
#endif
	return res;
}

uint8_t HAL_IMU_GET_Angle(float *angle)
{
#ifdef STM32F40_41xxx
	extern float JY60_Get_IncAsync();
	if (angle != 0)
	{
		*angle = JY60_Get_IncAsync();
	}
#else
	if (angle != 0)
	{
		*angle = 1.335f;
	}
#endif

	return 1;
}

uint8_t HAL_Set_UI_Page1_Msg(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	extern char *UI_Page1_Get_Msg_Array(uint16_t * len);
	extern void UI_Page1_Set_Msg_Finish(void);
	char *msg_pt = UI_Page1_Get_Msg_Array(0);
	while (msg_pt == 0)
	{
		msg_pt = UI_Page1_Get_Msg_Array(0);
	}
	vsprintf(msg_pt, format, args);
	va_end(args);
	UI_Page1_Set_Msg_Finish();
	return 1;
}

uint8_t HAL_Get_UI_MouseName(char *str, uint16_t size)
{
	return Message_Center_Read_prinft("page1", str, size, "MouseName?");
}

uint16_t HAL_Task_GetArray(Task_Parameter_Struct *taskarray, uint16_t arraySize)
{
	uint16_t res = Task_Stroage_GetArray(taskarray, arraySize);
	return res;
}

uint8_t HAL_FAN_INIT()
{
#ifdef STM32F40_41xxx
	FAN_Init();
#endif
}

uint8_t HAL_FAN_Set(uint8_t pwm_precent)
{
#ifdef STM32F40_41xxx
	FAN_Ctrl(pwm_precent);
#endif
}

uint8_t HAL_API_INIT(void)
{
	HAL_CAM_Init();
	HAL_CAM_SET_Set();
	HAL_FAN_INIT();
	return 0;
}
