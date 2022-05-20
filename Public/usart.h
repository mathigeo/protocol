#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h" 
#include "stdio.h" 


#define READ_COM(type,addr)  (*((type *)addr))

extern uint8_t comBuf[256];
extern uint8_t comLen;

void usart_init(u32 bound);
void usart_sendByte( USART_TypeDef * pUSARTx, uint8_t ch);
void usart_sendString( USART_TypeDef * pUSARTx, char *str);
void usart_sendData(USART_TypeDef * pUSARTx,uint8_t *base,uint8_t lenth);

#endif //__USART_H


