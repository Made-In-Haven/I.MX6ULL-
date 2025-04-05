#include "bsp_key_filter.h"
#include "bsp_int.h"
#include "bsp_gpio.h"
#include "bsp_beep.h"

#define delay_10  66000000/100  //10ms

void keyFilter_init()
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
	system_register_irqhandler(GPIO1_Combined_16_31_IRQn,gpio1_key_irqhandler,NULL);
	gpio_int_enable(GPIO1,18);
    filterTimer_init(delay_10);
}

/*初始化EPIT1定时器*/
void filterTimer_init(unsigned int value)
{ 
    /*设置时钟源*/
    EPIT1->CR |= (0X1<<24);
    /*设置为set and forget mode*/
    EPIT1->CR |= (1<<1);
    
    /*配置EPIT1_CR寄存器*/  
    EPIT1->CR &= ~(0XFFF<<4);
    EPIT1->CR |= ((1<<2)|(1<<3));
    EPIT1->LR = value;
    EPIT1->CMPR = 0;

    /*初始化中断*/
    GIC_EnableIRQ(EPIT1_IRQn);
    system_register_irqhandler(EPIT1_IRQn, epit1_key_irqhandler,NULL);

    /*不能默认打开定时器，只有按键按下才能打开*/
}

void stop_epit1()
{
    EPIT1->CR &= ~(1<<0); 
}

void restart_epit1(unsigned int value)
{
    EPIT1->CR &= ~(1<<0); 
    EPIT1->LR = value;
    EPIT1->CR |= (1<<0);
}

/*定时器中断触发，说明按键有效，在中断处理函数中进行相应按键按下的操作*/
void epit1_key_irqhandler(unsigned int gicciar, void *param)
{
    static int state = beep_off;
    if(((EPIT1->SR)&(1))==1)
    {
        /*关闭EPIT1定时器*/
        stop_epit1();

        if(gpio_pin_read(GPIO1,18)==0)
	    {
            state = !state;
            beep_switch(state);
	    }
    }
	EPIT1->SR |= 1; 
}



/*按键中断处理函数*/
void gpio1_key_irqhandler(unsigned int gicciar, void *param)
{
	/*重启定时器*/
    restart_epit1(delay_10);
    /*清除中断标志位*/
	gpio_clean_int_flag(GPIO1,18);
}