#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "MCIMX6Y2.h"
#include "cc.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"

void led_on(void);
void led_off(void);
void led_init(void);

#endif