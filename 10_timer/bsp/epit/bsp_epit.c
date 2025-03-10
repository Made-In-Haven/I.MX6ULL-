#include "bsp_epit.h"
#include "bsp_int.h"
#include "bsp_led.h"

void epit1_init(unsigned int frac, unsigned int value)
{
    if(frac>4095)
    {
        frac = 4095;
    } 
    /*禁用EPIT1 EPIT1输出 中断*/
    EPIT1->CR &= ~((1<<0)|(0X3<<22)|(1<<2)); 
    /*设置时钟源*/
    EPIT1->CR &= ~(0X3<<24);
    EPIT1->CR |= (0X1<<24);
    /*清除中断标志位*/
    EPIT1->SR |= 1;
    /*设置为set and forget mode*/
    EPIT1->CR |= (1<<1);
    
    /*配置EPIT1_CR寄存器*/  
    EPIT1->CR &= ~(0XFFF<<4);
    EPIT1->CR |= ((1<<2)|(1<<3)|(frac<<4));
    EPIT1->LR = value;
    EPIT1->CMPR = 0;

    /*初始化中断*/
    GIC_EnableIRQ(EPIT1_IRQn);
    system_register_irqhandler(EPIT1_IRQn, epit1_irqhandler,NULL);

    /*使能EPIT*/
    EPIT1->CR |= (1<<0);

}

/*epit1中断处理函数*/
void epit1_irqhandler(unsigned int gicciar, void *param)
{
    static unsigned int state = LED_OFF;
    if(((EPIT1->SR)&(1))==0x1)  //判断中断是否发生
    {
        state = !state;
        led_switch(LED0, state);
    }
    EPIT1->SR |= (1<<0);

}