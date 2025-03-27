#ifndef __BSP__EXTI_H
#define __BSP__EXTI_H

#include "imx6ul.h"
#include "bsp_gpio.h"


void gpio1_io18_irqhandler(unsigned int gicciar, void *param);
void exti_init();

#endif