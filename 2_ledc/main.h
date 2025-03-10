#ifndef __MAIN_H
#define __MAIN_H

#define CCCM_CCGR0 *((volatile unsigned int*)0x020c4068)//时钟
#define CCCM_CCGR1 *((volatile unsigned int*)0x020c406C)
#define CCCM_CCGR2 *((volatile unsigned int*)0x020c4070)
#define CCCM_CCGR3 *((volatile unsigned int*)0x020c4074)
#define CCCM_CCGR4 *((volatile unsigned int*)0x020c4078)
#define CCCM_CCGR5 *((volatile unsigned int*)0x020c407c)
#define CCCM_CCGR6 *((volatile unsigned int*)0x020c4080)
#define SW_MUX_GPIO1_IO03_BASE *((volatile unsigned int*)0x020e0068)//复用寄存器
#define SW_PAD_GPIO1_IO03_BASE *((volatile unsigned int*)0x0209c004) //配置IO3的电气属性
/*
 * GPIO1相关寄存器地址
 */
#define GPIO1_DR *((volatile unsigned int*)0X0209C000)
#define GPIO1_GDIR *((volatile unsigned int*)0X0209C004)
#define GPIO1_PSR *((volatile unsigned int*)0X0209C008)
#define GPIO1_ICR1 *((volatile unsigned int*)0X0209C00C)
#define GPIO1_ICR2 *((volatile unsigned int*)0X0209C010)
#define GPIO1_IMR *((volatile unsigned int*)0X0209C014)
#define GPIO1_ISR *((volatile unsigned int*)0X0209C018)
#define GPIO1_EDGE_SEL *((volatile unsigned int*)0X0209C01C)

#endif