#ifndef __Slave_Vor_Ctrl_H_
#define __Slave_Vor_Ctrl_H_
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

    uint8_t VOR_Machine_Init(float freq, float vel, uint32_t count);
    uint8_t VOR_Machine_Get_Count(uint32_t *counterReq, uint32_t *CurrentCounter);
    uint8_t VOR_Machine_Stop(void);

#ifdef __cplusplus
}
#endif

#endif