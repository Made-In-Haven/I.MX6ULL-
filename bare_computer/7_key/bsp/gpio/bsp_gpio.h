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

typedef struct _gpio_pin_config
{
    _gpio_pin_direction_t direction;
    uint8_t outputLogic;
}_gpio_pin_config_t;

void gpio_init(GPIO_Type * base, int pin, _gpio_pin_config_t *config);
void gpio_pin_write(GPIO_Type * base, int pin, int value);
int gpio_pin_read(GPIO_Type * base, int pin);



#endif