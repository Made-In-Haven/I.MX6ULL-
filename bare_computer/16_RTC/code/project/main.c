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
#include "bsp_beep.h"
#include "bsp_key.h"
#include "bsp_int.h"
#include "bsp_exti.h"
#include "bsp_epit.h"
#include "bsp_key_filter.h"
#include "bsp_uart.h"
#include "stdio.h"
#include "bsp_lcd.h"
#include "bsp_lcdapi.h"
#include "font.h" 
#include "bsp_rtc.h"


unsigned int backColor[3] = {
	LCD_LIGHTBLUE, LCD_BLUE, LCD_YELLOW
};	//刷屏颜色数组

int main(void)
{
	int_init();
	clk_init();
	clk_enable();		/* 使能所有的时钟 			*/
	beep_init();
	led_init();			/* 初始化led 			*/
	delay_init();
	uart_init(UART1,115200);
	keyFilter_init();			/*使用EPIT1给按键消抖*/
	lcd_init();			//lcd初始化

	struct rtc_dateTime rtc_data;
	//将rtc_data设置为当前时间
	rtc_data.year = 2025;
	rtc_data.month = 3;
	rtc_data.day = 23;
	rtc_data.hour = 12;
	rtc_data.minute = 20;
	rtc_data.second = 0;

	// rtc_init(&rtc_data);
	rtc_enable();		//关闭主电源后RTC是否正常运行测试

	tftlcd_dev.forecolor = LCD_RED;
	tftlcd_dev.backcolor = LCD_WHITE;
	lcd_show_string(10,40,260,32,32,(char*)"iKun");

	u64 data = 0;

	while(1)			/* 死循环 				*/
	{	
		rtc_getDateTime(&rtc_data);
		printf("当前时间：%d年%d月%d日%d时%d分%d秒\r\n",rtc_data.year
													,rtc_data.month
													,rtc_data.day
													,rtc_data.hour
													,rtc_data.minute
													,rtc_data.second);
		data = rtc_read_second();
		printf("当前秒数:%d\r\n",(unsigned int)(data));

		delay_ms(6000);
		
	}

	return 0;
}
