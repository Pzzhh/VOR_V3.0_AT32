#ifndef __Slave1_Timer_H_
#define __Slave1_Timer_H_
#include "stdint.h"

#ifdef __cplusplus
extern "C"
{
#endif
#define motor_step 10000 * 10
#define angle_step (277.8 * 5)
    typedef uint8_t (*Slave_Function_CB)(void);
    typedef enum
    {
        Slave_CMD_N,   // normal_mode
        Slave_CMD_Sin, // sin
        Slave_CMD_Back_N,
        Slave_CMD_Back_Sin,
    } Slave1_CMD_Typed;
    void Slave1_Step_Generator_Init(uint32_t arr, uint32_t psc);
    void Slave1_Set_Machine_Cb(Slave_Function_CB cb);
    void tim_f_sin_set(int f);
    void tim_f_set(int f);
    void Slave_server_Init(void);
    float Slave1_Get_Encode_Angle(void);
    int32_t Slave1_Get_Encode(void);
    void Slave1_Set_Encode(uint32_t cnt);

#ifdef __cplusplus
}
#endif

#endif