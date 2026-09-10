#ifndef __TCP_PROTOCOL_SERVER_H_
#define __TCP_PROTOCOL_SERVER_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*TcpProtocol_ReceiveCallback)(uint8_t data);
uint8_t TcpProtocol_Start(uint16_t port, TcpProtocol_ReceiveCallback receive_cb);
void TcpProtocol_SendBuf(uint8_t* data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
