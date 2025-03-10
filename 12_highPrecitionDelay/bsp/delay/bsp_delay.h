#ifndef __BSP_DELAY_H
#define __BSP_DELAY_H

#include "imx6ul.h"
void delay(volatile unsigned int n);
void delay_init();
void gpt1_irqhandler(unsigned int gicciar, void *param);


#endif