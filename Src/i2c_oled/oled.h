#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"


#define OLED_W_SCL(x)		GPIO_WriteBit(GPIOC,GPIO_Pin_10,(BitAction)(x))
#define OLED_W_SDA(x)		GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction)(x))


void OLED_Init(void);
void OLED_Clear(void);
void OLED_SetCursor(uint8_t Y, uint8_t X);
void OLED_WriteCommand(uint8_t Command);
void OLED_ShowChar(uint8_t row,uint8_t col,char Chr,uint8_t inv);
void OLED_ShowString(uint8_t row,uint8_t col,char *str,uint8_t inv);
int OLED_Print(u8 row,u8 col,const char *formate,...);
int loadbar(u8 page,u8 percent);

#endif /*__OLED_H*/


