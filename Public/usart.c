#include "usart.h"		
#include "string.h"


/**
 * @brief  Retargets the C library printf function to the USART.
 * @param  None
 * @retval None
 */
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0){}; 
    USART1->DR = (u8) ch;      
	return ch;
}




/**
 * @brief USART1初始化函数
 * @param bound 波特率
 * @retval None
 */
void usart_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	

	
	// 打开串口 GPIO 的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// 打开串口外设的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	
	
	// 将 USART Tx 的 GPIO 配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// 将 USART Rx 的 GPIO 配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
	
	// 配置波特率
	USART_InitStructure.USART_BaudRate = bound;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(USART1, &USART_InitStructure);
	
	
	
	
	/* 配置 USART 为中断源 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	/* 抢断优先级为 1 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	/* 子优先级为 1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	/* 使能中断 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* 初始化配置 NVIC */
	NVIC_Init(&NVIC_InitStructure);	
	
	
	
	// 使能串口接收中断
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	// 使能串口
	USART_Cmd(USART1,ENABLE);
}



/**
 * @brief 发送一个字节
 * @param pUSARTx 串口结构体
 * @param ch 字节数据
 * @retval None
 */
void usart_sendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* 发送一个字节数据到 USART */
	USART_SendData(pUSARTx,ch);

	/* 等待发送数据寄存器为空 */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}




/**
 * @brief 发送一个字符串
 * @param pUSARTx 串口结构体
 * @param str 字符串
 * @retval None
 */
void usart_sendString(USART_TypeDef * pUSARTx, char *str)
{
	unsigned int k=0;
	do{
		usart_sendByte( pUSARTx, *(str + k) );
		k++;
	}while (*(str + k)!='\0');

	/* 等待发送完成 */
	while (USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET) {
	}
}

//变量的定义
uint8_t comBuf[256] = {0};
uint8_t comLen = 0;



/**
 * @brief 带协议发送数据
 * @param pUSARTx 要发送的串口
 * @param base 数据首地址
 * @param lenth 数据长度(单位：字节)
 * @retval None
 */
void usart_sendData(USART_TypeDef * pUSARTx,uint8_t *base,uint8_t lenth)
{
	uint8_t i,xort=0;
	
	//发送帧头
	while((pUSARTx->SR&0X40)==0);
	pUSARTx->DR = (u8)(0X3A);
	while((pUSARTx->SR&0X40)==0);
	pUSARTx->DR = (u8)(0X5C);
	
	//发送数据长度
	while((pUSARTx->SR&0X40)==0);
	pUSARTx->DR = (u8)(lenth);
	
	//发送数据并不断异或得到校验码
	for(i=0;i<lenth;i++)
	{
		/* 发送一个字节数据到 USART */
		while((pUSARTx->SR&0X40)==0);
		/* 等待发送数据寄存器为空 */
		pUSARTx->DR = (u8)base[i];
		xort ^= base[i];
	}
	
	//发送校验码
	while((pUSARTx->SR&0X40)==0);
	pUSARTx->DR = (u8)(xort);
	
	/* 等待发送完成 */
	while (USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);
}



/**
 * @brief 协议处理函数
 * @param None
 * @retval None
 */
void protocol(uint8_t temp)
{
	static uint8_t step = 0;
	static uint8_t buf[256]={0};
	static uint8_t index = 0;
	static uint8_t len = 0;
	static uint8_t xort = 0;

	switch(step)
	{
		case 0:
		{
			//接收到帧头第一个8位
			if(temp == 0X3A) 
			{
				step = 1;
			}
			break;
		}
		case 1:
		{
			//接收到帧头第二个8位
			if(temp == 0X5C)
			{
				step = 2;
			}
			else
			{
				step = 0;
			}
			break;
		}
		case 2:
		{
			//接收数据长度
			len = temp;
			index = 0;
			xort = 0;
			step = 3;
			
			//如果长度为0
			if(len == 0)
			{
				//异常
				step = 0;
				len = 0;
				xort = 0;
				index = 0;
			}
			break;
		}
		case 3:
		{
			//接收数据
			if(index < len)
			{
				buf[index] = temp;
				xort ^= temp;
				index++;
			}
			else
			{
				//校验码
				if(xort == temp)
				{
					comLen = len;
					memcpy(comBuf,buf,len);
				}
				step = 0;
				len = 0;
				xort = 0;
				index = 0;
			}
			break;
		}
		default:
		{
			//异常
			step = 0;
			len = 0;
			xort = 0;
			index = 0;
		}
	}
}



/**
 * @brief USART1中断函数
 * @param None
 * @retval None
 */
void USART1_IRQHandler(void)
{
	static uint8_t status = 0;
	uint8_t temp;
	
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET) 
	{
		temp = USART_ReceiveData(USART1);
		//空闲状态
		if(status == 0)
		{
			//如果接收到0X66请求(通信请求)
			if(temp == 0X66)
			{
				//回复0X99作为应答
				while((USART1->SR&0X40)==0);
				USART1->DR = (u8)(0X99);	
				//换到接收态
				status = 1;				
			}
		}
		//工作状态
		else
		{
			protocol(temp);
		}
	}
}
