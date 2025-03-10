#ifndef __BSP_DELAY_H
#define __BSP_DELAY_H

#include "imx6ul.h"
void delay_init();
void gpt1_irqhandler(unsigned int gicciar, void *param);
void delay_ms(unsigned int number);
void delay_us(unsigned int number);


#endif