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
		OLED_Print(1,1,"%6.2f",READ_COM(float,comBuf));
	} 
}

