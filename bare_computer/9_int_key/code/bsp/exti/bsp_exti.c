#include "bsp_exti.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_delay.h"
#include "bsp_beep.h"
#include "bsp_led.h"

/*初始化外部中断*/
void exti_init()
{
    _gpio_pin_config_t key_config;
	
	/* 1、初始化IO复用, 复用为GPIO1_IO18 */
	IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18,0);

	/* 2、、配置UART1_CTS_B的IO属性	
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 11 默认22K上拉
	 *bit [13]: 1 pull功能
	 *bit [12]: 1 pull/keeper使能
	 *bit [11]: 0 关闭开路输出
	 *bit [7:6]: 10 速度100Mhz
	 *bit [5:3]: 000 关闭输出
	 *bit [0]: 0 低转换率
	 */
	IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18,0xF080);
	
	/* 3、初始化GPIO */
	key_config.interruptMode = kgpio_failing_edge;
	key_config.direction = kGPIO_Digitalinput;
	gpio_init(GPIO1,18, &key_config);
	GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);
	system_register_irqhandler(GPIO1_Combined_16_31_IRQn,gpio1_io18_irqhandler,NULL);
	gpio_int_enable(GPIO1,18);
}



/*按键中断处理函数*/
void gpio1_io18_irqhandler(unsigned int gicciar, void *param)
{
	static int state = beep_off;
	delay(10);  /*原则上不能加延时，中断要快进快出，暂时这样，原则上禁止在中断中调用延时*/
	if(gpio_pin_read(GPIO1,18)==0)
	{
		state = !state;
		beep_switch(state);
	}
	gpio_clean_int_flag(GPIO1,18);

}