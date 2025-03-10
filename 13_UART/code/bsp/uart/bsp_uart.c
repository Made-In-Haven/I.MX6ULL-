#include "bsp_uart.h"
#include "bsp_gpio.h"
/*UART初始化函数，波特率设置为115200*/
void uart_init(UART_Type* uart_base)
{
    //初始化IO
    uart1_gpio_init();
    //关闭串口
    uart_disable(uart_base);
    uart_reset(uart_base);
    //设置时钟
    CCM->CSCDR1 &= ~(1<<6);
    CCM->CSCDR1 &= ~(0X3F);
    //设置uart的数据位，奇偶校验，停止位
    uart_base->UCR2 = 0;
    uart_base->UCR2 |= (1<<5)|(1<<14);

    uart_base->UCR3 |= (1<<2);
    //设置波特率
    uart_base->UFCR &= ~(7<<7);
    uart_base->UFCR |= (5<<7); //对80Mhz进行1分频

    uart_base->UBIR &= ~(0xffff);
    uart_base->UBIR |= (71);
    uart_base->UBMR &= ~(0xffff);
    uart_base->UBMR |= (3124);

    //使能串口
    uart_base->UCR2 |= (1<<1)|(1<<2);   //收发使能
    uart_enable(uart_base);
}

/*uart1IO初始化函数*/
void uart1_gpio_init()
{
	/* 1、初始化IO复用*/
	IOMUXC_SetPinMux(IOMUXC_UART1_TX_DATA_UART1_TX,0);
    IOMUXC_SetPinMux(IOMUXC_UART1_RX_DATA_UART1_RX,0);

	IOMUXC_SetPinConfig(IOMUXC_UART1_TX_DATA_UART1_TX,0X10B0);  //设置为输出
    IOMUXC_SetPinConfig(IOMUXC_UART1_RX_DATA_UART1_RX,0xF080);  //设置为输入
    
}

/*关闭串口UART函数*/
void uart_disable(UART_Type* uart_base)
{
    uart_base->UCR1 &= ~(1<<0);
}

/*打开串口函数*/
void uart_enable(UART_Type* uart_base)
{
    uart_base->UCR1 |= (1<<0);
}

/*uart软复位函数*/
void uart_reset(UART_Type* uart_base)
{
    uart_base->UCR2 &= ~(1<<0);
    while(!((uart_base->UCR2&0x1)&0x1));

}