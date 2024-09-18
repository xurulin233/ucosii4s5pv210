#include "uart.h"
#include "led.h"
#include "stdio.h"
int main(void)
{
	led4();
	uart_init();
	printf("main init\n");
	led_blink();

}