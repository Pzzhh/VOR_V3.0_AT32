#ifndef __HOST_CONTROL_PROTOCOL_H_
#define __HOST_CONTROL_PROTOCOL_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*HostControl_SendCallback)(uint8_t* data, uint16_t len);

void HostControl_Init(HostControl_SendCallback send_cb);
void HostControl_ReceiveByte(uint8_t data);
void HostControl_Process(void);

#ifdef __cplusplus
}
#endif

#endif
