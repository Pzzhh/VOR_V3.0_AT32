
#ifndef __verfiy_h_
#define __verfiy_h_
#ifdef __cplusplus
extern "C"
{
#endif //
#include "stdint.h"
#include "../control/control_Hardware_API.h"
#ifndef ATM32F43_xx
#include "Windows.h"
#include <stdio.h>
#endif // use_windows
    typedef struct {
        timetyped licenseDate;
        uint8_t licenseValid;
        uint8_t timeConfigured;
    } Verify_Cache_Type;
    /* Access through the synchronized authorization functions. */
    extern Verify_Cache_Type Verify_Cache;
    void Verify_Init(void);
    void Verify_Config_Update(uint8_t id, const uint8_t *data, int length);
    uint8_t Verify_Compare_time(timetyped *expiredTag, timetyped localtime);
	uint8_t Verify_Check_isExpired(void);
	void Verify_hardResetVaild(void);
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
