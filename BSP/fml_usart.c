#include "fml_usart.h"
#include "fml_gpio.h"
#include "stdio.h"
#include "string.h"

#define USART1_IT_ENABLE	(1)
#define USART_BAUDRATE		(9600)

usart_data_t fml_uartCache;

void fml_usart_init(void)
{
	//结构体变量定义
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	USART_InitTypeDef USART_InitStruct = {0};
	
	//串口初始化配置
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	USART_InitStruct.USART_BaudRate = USART_BAUDRATE;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1,&USART_InitStruct);
	
	//中断配置
	#if USART1_IT_ENABLE
	USART_ClearITPendingBit(USART1,USART_IT_RXNE);
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn,NVIC_EncodePriority(5,0,0));
	#endif
	USART_Cmd(USART1,ENABLE);                                //启动串口
	
	//GPIO管脚配置
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);

	
}

uint8_t fml_usart_data_st(void)
{
	return fml_uartCache.recvflag;
}

void fml_usart_data_clear(void)
{
	memset(&fml_uartCache,0,sizeof(fml_uartCache));
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
}

void USART1_IRQHandler(void)
{
	uint8_t temp = 0;
	
	if (USART_GetITStatus(USART1,USART_IT_RXNE))
	{
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);
		temp = USART_ReceiveData(USART1);
		
		if ((fml_uartCache.recvLen == 0) && (temp != 0x55))
		{
			memset(&fml_uartCache,0,sizeof(fml_uartCache));
			return;
		}
		else if (fml_uartCache.recvLen == 4)
		{
			USART_ITConfig(USART1,USART_IT_RXNE,DISABLE);
			fml_uartCache.recvflag = 1;
		}
		fml_uartCache.recvBuff[fml_uartCache.recvLen++] = temp;
	}
}

int fputc(int ch, FILE* file)
{	
	USART_ClearFlag(USART1,USART_FLAG_TC);
	USART_SendData(USART1,ch);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC) == 0)
	{
		/* 等待上一个数据发送完成 */
	}
	return ch;
}

int fgetc(FILE*file)
{
	 while(!(USART1->SR & 1<<5));
	 return USART1->DR;
}


/*
	55 01 01 01 ff
*/
uint8_t fml_usart_data_handle(void)
{
	uint8_t cmd = 0;
	
	if (fml_uartCache.recvBuff[1] != 0x01)
		return ID_ERR;
	
	switch(fml_uartCache.recvBuff[2])
	{
		case 0x01://led
			if (fml_uartCache.recvBuff[3] == 0x01)
				cmd = LED_ON;
			else if (fml_uartCache.recvBuff[3] == 0x00)
				cmd = LED_OFF;	
			else
				cmd = DATA_ERR;
		break;
		case 0x02://蜂鸣器
			if (fml_uartCache.recvBuff[3] == 0x01)
				cmd = BUZZER_ON;
			else if (fml_uartCache.recvBuff[3] == 0x00)
				cmd = BUZZER_OFF;
			else
				cmd = DATA_ERR;
		break;
		default:
			cmd = DATA_ERR;
		break;
	}
	return cmd;	
}

void UsartCtrlTask(void)
{
	uint8_t cmd = 0;
	
	if (fml_usart_data_st() != 1)
		return;
	
	cmd = fml_usart_data_handle();
	
	switch(cmd)
	{
		case LED_ON:
			LED_PutVal(Bit_RESET);
			printf("led on\r\n");
		break;
		case LED_OFF:
			LED_PutVal(Bit_SET);
			printf("led off\r\n");
		break;		
		case BUZZER_ON:
			BUZZER_PutVal(Bit_RESET);
			printf("buzzer on\r\n");
		break;		
		case BUZZER_OFF:
			BUZZER_PutVal(Bit_SET);
			printf("buzzer off\r\n");
		break;
	
	}
	fml_usart_data_clear();//清空数据缓冲区，打开接收中断
}





