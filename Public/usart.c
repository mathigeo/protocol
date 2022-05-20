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
 * @brief USART1��ʼ������
 * @param bound ������
 * @retval None
 */
void usart_init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	

	
	// �򿪴��� GPIO ��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	// �򿪴��������ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	
	
	// �� USART Tx �� GPIO ����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// �� USART Rx �� GPIO ����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
	
	// ���ò�����
	USART_InitStructure.USART_BaudRate = bound;
	// ���� �������ֳ�
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// ����ֹͣλ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// ����У��λ
	USART_InitStructure.USART_Parity = USART_Parity_No;
	// ����Ӳ��������
	USART_InitStructure.USART_HardwareFlowControl =USART_HardwareFlowControl_None;
	// ���ù���ģʽ���շ�һ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	// ��ɴ��ڵĳ�ʼ������
	USART_Init(USART1, &USART_InitStructure);
	
	
	
	
	/* ���� USART Ϊ�ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	/* �������ȼ�Ϊ 1 */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	/* �����ȼ�Ϊ 1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	/* ʹ���ж� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* ��ʼ������ NVIC */
	NVIC_Init(&NVIC_InitStructure);	
	
	
	
	// ʹ�ܴ��ڽ����ж�
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	// ʹ�ܴ���
	USART_Cmd(USART1,ENABLE);
}



/**
 * @brief ����һ���ֽ�
 * @param pUSARTx ���ڽṹ��
 * @param ch �ֽ�����
 * @retval None
 */
void usart_sendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* ����һ���ֽ����ݵ� USART */
	USART_SendData(pUSARTx,ch);

	/* �ȴ��������ݼĴ���Ϊ�� */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}




/**
 * @brief ����һ���ַ���
 * @param pUSARTx ���ڽṹ��
 * @param str �ַ���
 * @retval None
 */
void usart_sendString(USART_TypeDef * pUSARTx, char *str)
{
	unsigned int k=0;
	do{
		usart_sendByte( pUSARTx, *(str + k) );
		k++;
	}while (*(str + k)!='\0');

	/* �ȴ�������� */
	while (USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET) {
	}
}

//�����Ķ���
uint8_t comBuf[256] = {0};
uint8_t comLen = 0;



/**
 * @brief ��Э�鷢������
 * @param pUSARTx Ҫ���͵Ĵ���
 * @param base �����׵�ַ
 * @param lenth ���ݳ���(��λ���ֽ�)
 * @retval None
 */
void usart_sendData(USART_TypeDef * pUSARTx,uint8_t *base,uint8_t lenth)
{
	uint8_t i,xort=0;
	
	//����֡ͷ
	while((pUSARTx->SR&0X40)==0);
	pUSARTx->DR = (u8)(0X3A);
	while((pUSARTx->SR&0X40)==0);
	pUSARTx->DR = (u8)(0X5C);
	
	//�������ݳ���
	while((pUSARTx->SR&0X40)==0);
	pUSARTx->DR = (u8)(lenth);
	
	//�������ݲ��������õ�У����
	for(i=0;i<lenth;i++)
	{
		/* ����һ���ֽ����ݵ� USART */
		while((pUSARTx->SR&0X40)==0);
		/* �ȴ��������ݼĴ���Ϊ�� */
		pUSARTx->DR = (u8)base[i];
		xort ^= base[i];
	}
	
	//����У����
	while((pUSARTx->SR&0X40)==0);
	pUSARTx->DR = (u8)(xort);
	
	/* �ȴ�������� */
	while (USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);
}



/**
 * @brief Э�鴦����
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
			//���յ�֡ͷ��һ��8λ
			if(temp == 0X3A) 
			{
				step = 1;
			}
			break;
		}
		case 1:
		{
			//���յ�֡ͷ�ڶ���8λ
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
			//�������ݳ���
			len = temp;
			index = 0;
			xort = 0;
			step = 3;
			
			//�������Ϊ0
			if(len == 0)
			{
				//�쳣
				step = 0;
				len = 0;
				xort = 0;
				index = 0;
			}
			break;
		}
		case 3:
		{
			//��������
			if(index < len)
			{
				buf[index] = temp;
				xort ^= temp;
				index++;
			}
			else
			{
				//У����
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
			//�쳣
			step = 0;
			len = 0;
			xort = 0;
			index = 0;
		}
	}
}



/**
 * @brief USART1�жϺ���
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
		//����״̬
		if(status == 0)
		{
			//������յ�0X66����(ͨ������)
			if(temp == 0X66)
			{
				//�ظ�0X99��ΪӦ��
				while((USART1->SR&0X40)==0);
				USART1->DR = (u8)(0X99);	
				//��������̬
				status = 1;				
			}
		}
		//����״̬
		else
		{
			protocol(temp);
		}
	}
}
