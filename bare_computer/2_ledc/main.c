#include "main.h"

/*使能外设时钟*/

void clk_enable()
{
    CCCM_CCGR0 = 0xffffffff;
    CCCM_CCGR1 = 0xffffffff;
    CCCM_CCGR2 = 0xffffffff;
    CCCM_CCGR3 = 0xffffffff;
    CCCM_CCGR4 = 0xffffffff;
    CCCM_CCGR5 = 0xffffffff;
    CCCM_CCGR6 = 0xffffffff;
}

void led_enable()
{
    SW_MUX_GPIO1_IO03_BASE = 0x5;
    SW_PAD_GPIO1_IO03_BASE = 0x10b0;//输出
    GPIO1_GDIR = 0X8;//设置为输出
    GPIO1_DR = 0x0;//打开led
}


/*短延时*/
void delay_short(volatile unsigned int n)
{
    while(n--){}
}

/*延时,一次循环延时1ms，在主频396Mhz下

*/
void delay(volatile unsigned int n)
{
    while(n--)
    {
        delay_short(0x7ff);
    }
}


void led_on()
{
    GPIO1_DR &= ~(1<<3);
}

void led_off()
{
    GPIO1_DR |= 1<<3;
}



int main(void)
{
    /*初始化led*/
    clk_enable();
    led_enable();
    
    /*led闪烁*/


    while(1)
    {
        led_on();
        delay(500);
        led_off();
        delay(500);
    }
    return 0;
}