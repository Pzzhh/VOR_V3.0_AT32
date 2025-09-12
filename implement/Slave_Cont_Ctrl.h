/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-08-02 10:40:15
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @LastEditTime: 2024-08-02 11:08:28
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\implement\Slave_Cont_Ctrl.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __Slave_Vor_Ctrl_H_
#define __Slave_Vor_Ctrl_H_
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif

    uint8_t CONT_Machine_Init(float vel, uint32_t MillSec);
    uint8_t Cont_Machine_Stop(void);
    uint8_t Cont_Machine_Get_Count(uint32_t *MillSecReq, uint32_t *CurrentMillSec);
    
#ifdef __cplusplus
}
#endif

#endif