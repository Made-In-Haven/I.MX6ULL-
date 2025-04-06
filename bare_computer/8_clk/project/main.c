/**************************************************************
Copyright © zuozhongkai Co., Ltd. 1998-2019. All rights reserved.
文件名	: 	 mian.c
作者	   : 左忠凯
版本	   : V1.0
描述	   : I.MX6U开发板裸机实验3 STM32模式的LED灯开发
		 使用STM32开发模式来编写LED灯驱动，学习如何从STM32来转入
		 I.MX6U的裸机开发，同时也通过本实验了解STM32库的运行方式。
其他	   : 无
日志	   : 初版V1.0 2019/1/3 左忠凯创建
**************************************************************/
#include "main.h"
#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_led.h"
//#include "bsp_beep.h"
#include "bsp_key.h"


int main(void)
{
	int i=0;
	unsigned char led_status = OFF;
	clk_init();
	clk_enable();		/* 使能所有的时钟 			*/
	led_init();			/* 初始化led 			*/
	key_init();


	while(1)			/* 死循环 				*/
	{	
		
		// led_on();
		// symbol = key_getValue();
		// if(symbol == KEY0_VALUE)
		// {
		// 	led_off();
		// 	delay(500);
		// }
		i++;
		if(i==50)
		{
			i=0;
			led_status = !led_status;
			led_switch(LED0, led_status);
		}
		delay(10);
	}

	return 0;
}
