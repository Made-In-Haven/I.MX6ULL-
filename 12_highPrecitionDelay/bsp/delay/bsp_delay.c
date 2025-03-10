#include "bsp_delay.h"
#include "bsp_int.h"
#include "bsp_led.h"
void delay_short(volatile unsigned int n)
{
	while(n--){}
}

/*
 * @description	: 延时函数,在396Mhz的主频下
 * 			  	  延时时间大约为1ms
 * @param - n	: 要延时的ms数
 * @return 		: 无
 */
void delay(volatile unsigned int n)
{
	while(n--)
	{
		delay_short(0x7ff);
	}
}


/*使用通用定时器GPT延时*/
void delay_init()
{
	GPT1->CR &= ~(1<<0);//打开gpt1
	GPT1->CR = 1 << 15;				/* bit15置1进入软复位 				*/
	while(((GPT1->CR)>>15 & (1))); //等待复位结束
	GPT1->CR |= (1<<1); //GPT counter value is reset to 0 when it is disabled.

	GPT1->CR &= ~(1<<9);  //restart mode

	GPT1->CR &= ~(0X7<<6);
	GPT1->CR |= (1<<6);  //choose ipg_clk

	GPT1->PR &= ~(0XFFF);
	GPT1->PR |= (65<<0); //66分频  1MHZ

	GPT1->OCR[0] = 1000000/2; //设置中断周期是500ms
	while(((GPT1->CR)>>15 & (1))); //等待复位结束
		
	GPT1->IR |= (1<<0);	 //Output Compare Channel 1 interrupt is enabled.
	GIC_EnableIRQ(GPT1_IRQn);
	system_register_irqhandler(GPT1_IRQn,gpt1_irqhandler,NULL);

	GPT1->CR |= (1<<0);//打开gpt1
	

}


void gpt1_irqhandler(unsigned int gicciar, void *param)
{	
	static unsigned int led_state = LED_ON;
	if((GPT1->SR&(0X1)))
	{
		led_switch(LED0,led_state);
		led_state = !led_state;
	}
	GPT1->SR |= 1;  //清除中断标志位
}