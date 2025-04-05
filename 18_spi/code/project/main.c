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
#include "bsp_ap3216c.h"
#include "bsp_i2c.h"
#include "bsp_spi.h"
#include "bsp_icm20608.h"

void integer_display(unsigned short x, unsigned short y, unsigned char size, signed int num);
void decimals_display(unsigned short x, unsigned short y, unsigned char size, signed int num);
void imx6ul_hardfpu_enable(void);




int main(void)
{
	int_init();
	imx6ul_hardfpu_enable();	//开启硬件浮点计算及neon
	clk_init();
	clk_enable();		/* 使能所有的时钟 			*/
	beep_init();
	led_init();			/* 初始化led 			*/
	delay_init();
	uart_init(UART1,115200);
	keyFilter_init();			/*使用EPIT1给按键消抖*/
	lcd_init();			//lcd初始化
	// ap3216c_init(I2C1);
	icm20608_init();

	tftlcd_dev.forecolor = LCD_RED;
	tftlcd_dev.backcolor = LCD_WHITE;
	lcd_show_string(10,40,260,32,32,(char*)"iKun");	
	tftlcd_dev.forecolor = LCD_BLUE;	
	while(icm20608_init())		/* 初始化ICM20608	 			*/
	{
		lcd_show_string(50, 100, 200, 16, 16, (char*)"ICM20608 Check Failed!");
		delay_ms(500);
		lcd_show_string(50, 100, 200, 16, 16, (char*)"Please Check!        ");
		delay_ms(500);
	}	

	lcd_show_string(50, 100, 200, 16, 16, (char*)"ICM20608 Ready");
	
	lcd_show_string(50, 130, 200, 16, 16, (char*)"accel x:");  
	lcd_show_string(50, 150, 200, 16, 16, (char*)"accel y:");  
	lcd_show_string(50, 170, 200, 16, 16, (char*)"accel z:");  
	lcd_show_string(50, 190, 200, 16, 16, (char*)"gyro  x:"); 
	lcd_show_string(50, 210, 200, 16, 16, (char*)"gyro  y:"); 
	lcd_show_string(50, 230, 200, 16, 16, (char*)"gyro  z:"); 
	lcd_show_string(50, 250, 200, 16, 16, (char*)"temp   :"); 

	lcd_show_string(50 + 181, 130, 200, 16, 16, (char*)"g");  
	lcd_show_string(50 + 181, 150, 200, 16, 16, (char*)"g");  
	lcd_show_string(50 + 181, 170, 200, 16, 16, (char*)"g");  
	lcd_show_string(50 + 181, 190, 200, 16, 16, (char*)"o/s"); 
	lcd_show_string(50 + 181, 210, 200, 16, 16, (char*)"o/s"); 
	lcd_show_string(50 + 181, 230, 200, 16, 16, (char*)"o/s"); 
	lcd_show_string(50 + 181, 250, 200, 16, 16, (char*)"C");
	
	tftlcd_dev.forecolor = LCD_BLUE;

	while(1)					
	{		
		icm20608_get_data();
		integer_display(50 + 70, 130, 16, icm20608_dev.accel_x_adc);
		integer_display(50 + 70, 150, 16, icm20608_dev.accel_y_adc);
		integer_display(50 + 70, 170, 16, icm20608_dev.accel_z_adc);
		integer_display(50 + 70, 190, 16, icm20608_dev.gyro_x_adc);
		integer_display(50 + 70, 210, 16, icm20608_dev.gyro_y_adc);
		integer_display(50 + 70, 230, 16, icm20608_dev.gyro_z_adc);
		integer_display(50 + 70, 250, 16, icm20608_dev.temp_adc);

		decimals_display(50 + 70 + 50, 130, 16, icm20608_dev.accel_x_act);
		decimals_display(50 + 70 + 50, 150, 16, icm20608_dev.accel_y_act);
		decimals_display(50 + 70 + 50, 170, 16, icm20608_dev.accel_z_act);
		decimals_display(50 + 70 + 50, 190, 16, icm20608_dev.gyro_x_act);
		decimals_display(50 + 70 + 50, 210, 16, icm20608_dev.gyro_y_act);
		decimals_display(50 + 70 + 50, 230, 16, icm20608_dev.gyro_z_act);
		decimals_display(50 + 70 + 50, 250, 16, icm20608_dev.temp_act);	//这里的温度是芯片内部的温度，不是环境温度

#if 0		
		printf("accel x = %d\r\n",icm20608_dev.accel_x_adc);
		printf("accel y = %d\r\n",icm20608_dev.accel_y_adc);
		printf("accel z = %d\r\n",icm20608_dev.accel_z_adc);
		printf("gyrp  x = %d\r\n",icm20608_dev.gyro_x_adc);
		printf("gyro  y = %d\r\n",icm20608_dev.gyro_y_adc);
		printf("gyro  z = %d\r\n",icm20608_dev.gyro_z_adc);
		printf("temp    = %d\r\n",icm20608_dev.temp_adc);	
#endif
		delay_ms(120);
	}
	return 0;
}
/*
 * @description	: 指定的位置显示整数数据
 * @param - x	: X轴位置
 * @param - y 	: Y轴位置
 * @param - size: 字体大小
 * @param - num : 要显示的数据
 * @return 		: 无
 */
void integer_display(unsigned short x, unsigned short y, unsigned char size, signed int num)
{
	char buf[200];
	
	lcd_fill(x, y, x + 50, y + size, tftlcd_dev.backcolor);
	
	memset(buf, 0, sizeof(buf));
	if(num < 0)
		snprintf(buf, sizeof(buf), "%d", num);
	else 
		snprintf(buf, sizeof(buf), "%d", num);
	lcd_show_string(x, y, 50, size, size, buf); 
}


/*
 * @description	: 指定的位置显示小数数据,比如5123，显示为51.23
 * @param - x	: X轴位置
 * @param - y 	: Y轴位置
 * @param - size: 字体大小
 * @param - num : 要显示的数据，实际小数扩大100倍，
 * @return 		: 无
 */
void decimals_display(unsigned short x, unsigned short y, unsigned char size, signed int num)
{
	signed int integ; 	/* 整数部分 */
	signed int fract;	/* 小数部分 */
	signed int uncomptemp = num; 
	char buf[200];

	if(num < 0)
		uncomptemp = -uncomptemp;
	integ = uncomptemp / 100;	//数据在传入函数之前已经被放大了100倍
	fract = uncomptemp % 100;	//数据在传入函数之前已经被放大了100倍

	memset(buf, 0, sizeof(buf));
	if(num < 0)
		snprintf(buf, sizeof(buf), "-%d.%d", integ, fract);
	else 
		snprintf(buf, sizeof(buf), "%d.%d", integ, fract);
	lcd_fill(x, y, x + 60, y + size, tftlcd_dev.backcolor);
	lcd_show_string(x, y, 60, size, size, buf); 
}

/*
 * @description	: 使能I.MX6U的硬件NEON和FPU
 * @param 		: 无
 * @return 		: 无
 */
 void imx6ul_hardfpu_enable(void)
{
	uint32_t cpacr;
	uint32_t fpexc;

	/* 使能NEON和FPU */
	cpacr = __get_CPACR();
	cpacr = (cpacr & ~(CPACR_ASEDIS_Msk | CPACR_D32DIS_Msk))
		   |  (3UL << CPACR_cp10_Pos) | (3UL << CPACR_cp11_Pos);
	__set_CPACR(cpacr);
	fpexc = __get_FPEXC();
	fpexc |= 0x40000000UL;	
	__set_FPEXC(fpexc);
}
