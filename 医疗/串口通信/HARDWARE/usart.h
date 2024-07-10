#include "sys.h"
#include "stdio.h"

#ifndef __USART_H
#define __USART_H

#define USART_REC_LEN 200
#define EN_USART_RX 1
#define EN_USART_RX3 1

extern uint16_t USART_RX_BUF[USART_REC_LEN];  

extern u16 USART_RX_STA;


void uart_init(u32 bound);//初始化串口函数
void uart3_init(u32 bound);//初始化串口3函数

void send(u16 data);
void send3(int data);

#endif

