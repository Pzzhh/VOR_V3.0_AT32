#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <setjmp.h>
#define ATM32F43_xx
#define Task_ARY_MAX 15
#include "INTERFACE/UI/control/control.h"
#include "INTERFACE/UI/varify/verfiy.h"
#define taskENTER_CRITICAL() ((void)0)
#define taskEXIT_CRITICAL() ((void)0)
#define portMAX_DELAY 0xffffffff
#define pdTRUE 1
#define Rk3588_Task_Cancel do { control->Rk3588_Flag.Lflag = Cancel; control->Rk3588_Flag.Rflag = Cancel; } while (0)
typedef void *SemaphoreHandle_t;
static int mutex_failed;
static SemaphoreHandle_t xSemaphoreCreateMutex(void) { return mutex_failed ? NULL : (void *)1; }
static int xSemaphoreTake(SemaphoreHandle_t s, unsigned long wait) { return 1; }
static void xSemaphoreGive(SemaphoreHandle_t s) {}
#define EROM_MAP_info_pS 32
#define EROM_MAP_info_base 0
#define EROM_MAP_Task_Stroage 0x300
static uint8_t eeprom[0x300];
static int read_failed, write_failed, read_calls, task_loads, movements, zero_calls;
static timetyped current_time;
static int rtc_failed;
Task_control_info control_info;
static Task_control_info *control;
Task_Parameter_Struct TaskArray[Task_ARY_MAX];
static jmp_buf handler_exit;
static int idle_calls;
static int expire_pending;
static timetyped pending_time;
static uint16_t crc16_modbus(const uint8_t *data, unsigned int length)
{
    uint16_t crc = 0xffff;
    while (length--) {
        crc ^= *data++;
        for (int i = 0; i < 8; ++i) crc = (crc >> 1) ^ ((crc & 1) ? 0xa001 : 0);
    }
    return crc;
}
static int M24C64_Read_Byte(uint16_t addr, uint8_t *buf, uint16_t len)
{
    ++read_calls;
    if (read_failed) return 1;
    memcpy(buf, eeprom + addr, len);
    return 0;
}
static int M24C64_Write_Byte(uint16_t addr, uint8_t *buf, uint16_t len)
{
    if (write_failed) return 1;
    memcpy(eeprom + addr, buf, len);
    return 0;
}
uint8_t HAL_TIME_Get(timetyped *t) { *t = current_time; return !rtc_failed; }
uint8_t HAL_TIME_Set(timetyped *t) { current_time = *t; return 1; }
uint16_t HAL_Task_GetArray(Task_Parameter_Struct *array, uint16_t size)
{ ++task_loads; return 1; }
uint8_t Ctrl_Msg_Printf(const char *format, ...)
{
    va_list args; va_start(args, format);
    vsnprintf((char *)control_info.message, sizeof(control_info.message), format, args);
    va_end(args); return 1;
}
static void SaftExitDelay(int delay, int ignored)
{
    if (++idle_calls == 1) {
        control_info.State_Bit.isRun = 1;
        if (expire_pending) current_time = pending_time;
    } else longjmp(handler_exit, 1);
}
static void ZEROControlFunction(int start) { ++zero_calls; longjmp(handler_exit, 1); }
static void VorControlFunction(Task_Parameter_Struct *t, Task_control_info *e) { ++movements; }
static void ContControlFunction(Task_Parameter_Struct *t, Task_control_info *e) { ++movements; }
static void OvarControlFunction(Task_Parameter_Struct *t, Task_control_info *e, int retract) { ++movements; }
static void TcControlFunction(Task_Parameter_Struct *t, Task_control_info *e) { ++movements; }
static void VHITControlFunction(Task_Parameter_Struct *t, Task_control_info *e) { ++movements; }
int HAL_Outage_Log(uint8_t flag, uint8_t index) { return 1; }
uint8_t HAL_Beep_Enable(uint32_t ms) { return 1; }
static void task_interval_handle(int i) {}
static uint32_t ControlGetTick(void) { return 0; }
static void HAL_USB_Log_Printf(const char *format, ...) {}
uint8_t Message_Center_Send_prinft(const char *name, void *a, uint8_t size, const char *format, ...) { return 0; }
