#ifndef HDL_USART_H
#define HDL_USART_H


#include "stm32f10x.h"
#include "stdio.h"


typedef struct{
	uint8_t recvBuff[10];	//数据接收缓冲区
	uint8_t recvflag;		//数据接收标志位
	uint8_t recvLen;		//数据接收长度
}usart_data_t;

typedef enum{
	ID_ERR = 1,
	DATA_ERR,
	LED_ON,
	LED_OFF,
	BUZZER_ON,
	BUZZER_OFF,
}usart_data_result_e;

extern usart_data_t fml_uartRecv;




void fml_usart_init(void);
uint8_t fml_usart_data_st(void);
void fml_usart_data_clear(void);
uint8_t fml_usart_data_handle(void);
void UsartCtrlTask(void);



#endif
