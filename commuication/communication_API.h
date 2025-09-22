/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-30 11:26:20
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @LastEditTime: 2024-07-31 11:41:10
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\commuication\communication_API.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AEuint8_t HAL_BL_USART_INIT(void)
 */
#ifndef  __communication_API_H
#define __communication_API_H

#ifdef __cplusplus
extern "C"
{
#endif //
#include "stdint.h"

	uint8_t HAL_BL_USART_Server(void);
	void HAL_communication_Init(void);
	int Hal_Rk3588_R_ReadLine(const char* format, ...);
	int Hal_Rk3588_L_ReadLine(const char* format, ...);
	//UART1 right
	int Hal_Rk3588_R_Readarray(char* bytes);
	//UART6 left
	int Hal_Rk3588_L_Readarray(char* bytes);
	int HAL_USB_Isconnect();

#ifdef __cplusplus
}
#endif //

#endif // ! __communication_API_H