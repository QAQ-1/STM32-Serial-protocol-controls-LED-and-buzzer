#include "stm32f10x.h"
#include "fml_gpio.h"
#include "fml_time.h"
#include "fml_usart.h"

int main(void)
{
	fml_led_init();
	fml_usart_init();	
	fml_buzzer_init();
	printf("hello world\r\n");
	for(;;)
	{
		UsartCtrlTask();	
	}
}




















