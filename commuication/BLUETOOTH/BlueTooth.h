#ifndef __BlueTooth_H_
#define __BlueTooth_H_
#include "stdint.h"

void usart_protocol_InterruptHandle(uint8_t dr);
void usart_protocol_decoding(uint8_t *bytes);
void usart_protocol_init(void (*cb)(uint8_t* buf, uint16_t len));
#endif