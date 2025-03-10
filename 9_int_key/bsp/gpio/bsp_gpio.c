#include "bsp_gpio.h"

void gpio_init(GPIO_Type * base, int pin, _gpio_pin_config_t *config)
{
    if(config->direction == kGPIO_Digitalinput)
    {
        base->GDIR &= ~(1 << pin);
    }
    else
    {
        base->GDIR |= (1 << pin);
        gpio_pin_write(base, pin, config->outputLogic);
    }
    if(config->interruptMode != kgpio_Nointmode)
    {
        gpio_int_init(base, pin, config->interruptMode);
    }
}

/*控制GPIO高低电平*/

void gpio_pin_write(GPIO_Type * base, int pin, int value)
{
    if(value==0)   //写0
    {
        base->DR &= ~(1<<pin);
    }
    else           //写1
    {
        base->DR |= (1<<pin);
    }
}

int gpio_pin_read(GPIO_Type * base, int pin)
{
    return ((base->DR)>>pin)&0x1;
}

/*GPIO中断初始化函数*/
void gpio_int_init(GPIO_Type* base, int pin, gpio_interrupt_mode mode)
{
    volatile uint32_t* ICR;
    uint32_t ICRshift;
    ICRshift = pin;
    base->EDGE_SEL &= ~(1<<pin);
    if(pin>=16)
    {
        ICR = &(base->ICR2);
        ICRshift -= 16;
    }
    else
    {
        ICR = &(base->ICR1);
    }
    switch (mode)  //设置中断触发方式
    {
    case kgpio_low_level:
        (*ICR) &= ~(0X3<<(ICRshift*2));
        break;
    case kgpio_high_level:
        (*ICR) &= ~(0X3<<(ICRshift*2));
        (*ICR) |= (1<<(ICRshift*2));
        break;
    case kgpio_rising_edge:
        (*ICR) &= ~(0X3<<(ICRshift*2));
        (*ICR) |= (2<<(ICRshift*2));
        break;
    case kgpio_failing_edge:
        (*ICR) &= ~(0X3<<(ICRshift*2));
        (*ICR) |= (3<<(ICRshift*2));
        break;
    case kgpio_risingOrFalling:
        base->EDGE_SEL |= (1<<pin);
        break;
    default:
        (*ICR) &= ~(0X3<<(ICRshift*2));
        break;
    }

}
/*中断使能*/
void gpio_int_enable(GPIO_Type* base, int pin)
{
    base->IMR |= (1<<pin);
}
/*禁止中断*/
void gpio_int_disenable(GPIO_Type* base, int pin)
{
    base->IMR &= ~(1<<pin);
}
/*清楚中断标志位*/
void gpio_clean_int_flag(GPIO_Type* base, int pin)
{
    base->ISR |= (1<<pin);
}
