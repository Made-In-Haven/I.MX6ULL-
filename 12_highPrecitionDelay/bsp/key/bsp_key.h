#ifndef __KEY_H
#define __KEY_H

#include "cc.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "MCIMX6Y2.h"
#include "bsp_delay.h"
#include "bsp_gpio.h"

enum keyValue{
    KEY_NONE = 0,
    KEY0_VALUE
};


void key_init();
int read_key();

#endif