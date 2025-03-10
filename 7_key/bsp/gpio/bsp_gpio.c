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