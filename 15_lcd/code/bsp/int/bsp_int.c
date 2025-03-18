#include "bsp_int.h"

/*中断嵌套*/
static int irqNesting;

/*中断处理函数表*/
static sys_irq_handle irqTable[NUMBER_OF_INT_VECTORS];

/*初始化中断处理函数表*/
void system_irqTable_init(void)
{
    unsigned int i=0;
    irqNesting = 0;
    for(i=0;i<NUMBER_OF_INT_VECTORS;i++)
    {
        irqTable[0].irqhandler = default_irqhandler;
        irqTable[0].userparam = NULL;
    }
}


/*注册中断处理函数*/
void system_register_irqhandler(IRQn_Type irq, system_irqhandler_t irqhandler, void* userParam)
{
    irqTable[irq].irqhandler = irqhandler;
    irqTable[irq].userparam = userParam;
}


/*中断初始化*/
void int_init()
{
    GIC_Init();

    /*中断向量偏移设置*/

    __set_VBAR((uint32_t)0X87800000);
    system_irqTable_init();

}

/*默认中断处理函数*/

void default_irqhandler (unsigned int gicciar, void *param)
{
    return;
}

/*通用中断处理函数*/
void system_irqhandler(unsigned int gicciar)
{
    /*检查中断ID是否正常*/
    unsigned int intNum = (gicciar&(0x3ff));
    if(intNum>=NUMBER_OF_INT_VECTORS)
    {
        return;
    }
    irqNesting++;
    /*根据中断ID，读取中断处理函数，执行*/
    irqTable[intNum].irqhandler(intNum,irqTable[intNum].userparam);
    irqNesting--;

}