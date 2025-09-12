
#ifndef __verfiy_h_
#define __verfiy_h_
#ifdef __cplusplus
extern "C"
{
#endif //
#include "stdint.h"
#ifndef STM32F40_41xxx
#include "Windows.h"
#include <stdio.h>
#endif // use_windows
	uint8_t Verify_Check_isExpired(void);
	uint8_t Verify_Set_Time(void* time);
	uint8_t Verify_Set_Subscribe(uint8_t month);
	uint8_t Verify_get_license_Date(timetyped* t);
	typedef enum
	{
		okay = 0,
		expired,
		NoTag,
		SetTimeOnly,
		SeTboth,
	}VarfiyTyped;

#ifdef __cplusplus
}
#endif //

#endif // ! UI_Paremeter_Set_H_
