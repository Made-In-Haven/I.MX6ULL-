#include "bsp_delay.h"
#include "bsp_int.h"
#include "bsp_led.h"

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
#if 0
	GPT1->OCR[0] = 1000000/2; //设置中断周期是500ms
	while(((GPT1->CR)>>15 & (1))); //等待复位结束
		
	GPT1->IR |= (1<<0);	 //Output Compare Channel 1 interrupt is enabled.
	GIC_EnableIRQ(GPT1_IRQn);
	system_register_irqhandler(GPT1_IRQn,gpt1_irqhandler,NULL);
#endif

	GPT1->OCR[0] = 0XFFFFFFFF;		//设置比较寄存器为最大值
	while(((GPT1->CR)>>15 & (1))); //等待复位结束
	GPT1->CR |= (1<<0);//打开gpt1
	
}
/*使用通用定时器实现微秒级延时,最低貌似只能到20us*/
void delay_us(unsigned int number)
{
	unsigned int oldcnt,newcnt;
	unsigned int tcntvalue = 0;

	oldcnt = GPT1->CNT;
	while(1)
	{
		newcnt = GPT1->CNT;
		if(newcnt!=oldcnt)		//代表开始延时
		{
			if(newcnt>oldcnt)	//正常情况
				tcntvalue = newcnt-oldcnt;
			else				//计数器溢出情况
				tcntvalue = 0xffffffff - oldcnt + newcnt;
			if(tcntvalue>=number)//延时时间到了
				break;
		}
	}
}
/*使用通用定时器实现毫秒级延时*/
void delay_ms(unsigned int number)
{
	unsigned int step = 0;

	for(step = 0;step<number;step++)
		delay_us(1000);
}

/*通用定时器中断实现led闪烁*/
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