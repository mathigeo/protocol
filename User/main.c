#include "system.h"  
#include "systick.h"
#include "usart.h"
#include "oled.h"



int main(void)
{		
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	OLED_Init();
	usart_init(115200);
	
	while(1)
	{
		OLED_Print(3,1,"%3d",READ_COM(short,comBuf));
	} 
}

