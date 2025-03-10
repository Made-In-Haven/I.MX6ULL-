#ifndef __BSP_KEY_FILTER_H
#define __BSP_KEY_FILTER_H

#include "imx6ul.h"

void keyFilter_init();
void filterTimer_init(unsigned int value);
void stop_epit1();
void restart_epit1(unsigned int value);
void epit1_key_irqhandler(unsigned int gicciar, void *param);
void gpio1_key_irqhandler(unsigned int gicciar, void *param);
#endif