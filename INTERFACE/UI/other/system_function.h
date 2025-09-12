/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-23 16:30:52
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @LastEditTime: 2024-07-31 11:53:16
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\INTERFACE\UI\other\system_function.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*
 * @Author: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @Date: 2024-07-22 16:00:07
 * @LastEditors: pzzhh2 101804901+Pzzhh@users.noreply.github.com.
 * @LastEditTime: 2024-07-23 15:56:20
 * @FilePath: \USERd:\workfile\项目3 vor\software\VOR_V2.0\INTERFACE\UI\other\list.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef _system_function_h_
#define _system_function_h_
#include "stdint.h"
#ifdef __cplusplus
extern "C"
{
#endif //

    void ControlDelay(uint32_t ms);
    void SaftExitDelay(uint32_t ms, uint8_t e);
    uint32_t ControlGetTick();
    void OutputDebugPrintf(const char* strOutputString, ...);
    long ControlThreadCreate(void *function,     // set thread code
                             void *e,            // set parameter
                             void *taskhandle_t, // thread instant
                             const char *name,   // thread name
                             uint16_t sizeofstack);
#ifdef __cplusplus
}
#endif //

#endif // LIST_H
