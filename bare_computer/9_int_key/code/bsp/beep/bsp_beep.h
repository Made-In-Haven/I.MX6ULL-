#ifndef __BEEP_H
#define __BEEP_H

#include "cc.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "MCIMX6Y2.h"



/*函数声明*/
void beep_init();
void beep_switch(int status);

#endif