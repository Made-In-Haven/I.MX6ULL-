#ifndef __INT_H
#define __INT_H
#include "imx6ul.h"
/*定义中断处理函数*/
typedef void (*system_irqhandler_t)(unsigned int gicciar, void *param);

/*中断函数处理结构体*/
typedef struct _sys_irq_handle{
    system_irqhandler_t irqhandler; /*中断处理函数*/
    void *userparam;                /*中断处理函数的参数*/


}sys_irq_handle;

void int_init();
void system_irqhandler(unsigned int gicciar);
void system_irqTable_init(void);
void default_irqhandler (unsigned int gicciar, void *param);
void system_register_irqhandler(IRQn_Type irq, system_irqhandler_t irqhandler, void* userParam);

#endif