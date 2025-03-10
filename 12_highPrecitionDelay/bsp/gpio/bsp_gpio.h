#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "cc.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "MCIMX6Y2.h"
#include "bsp_delay.h"

/*枚举类型和GPIO结构体*/
typedef enum _gpio_pin_direction
{
    kGPIO_Digitalinput = 0U,
    kGPIO_Digitaloutput = 1U
}_gpio_pin_direction_t;

/*描述中断触发类型*/
typedef enum _gpio_interrupt_mode{
    kgpio_Nointmode = 100U,
    kgpio_low_level = 0U,
    kgpio_high_level,
    kgpio_rising_edge,
    kgpio_failing_edge,
    kgpio_risingOrFalling
}gpio_interrupt_mode;

typedef struct _gpio_pin_config
{
    _gpio_pin_direction_t direction;
    uint8_t outputLogic;
    gpio_interrupt_mode interruptMode; /*中断触发方式*/
}_gpio_pin_config_t;

void gpio_init(GPIO_Type * base, int pin, _gpio_pin_config_t *config);
void gpio_pin_write(GPIO_Type * base, int pin, int value);
int gpio_pin_read(GPIO_Type * base, int pin);
void gpio_int_init(GPIO_Type* base, int pin, gpio_interrupt_mode mode);
void gpio_clean_int_flag(GPIO_Type* base, int pin);
void gpio_int_enable(GPIO_Type* base, int pin);
void gpio_int_disenable(GPIO_Type* base, int pin);



#endif