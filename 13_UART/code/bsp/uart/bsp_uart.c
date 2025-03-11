#include "bsp_uart.h"
#include "bsp_gpio.h"
#include "bsp_led.h"
/*UART初始化函数，波特率设置为115200*/
void uart_init(UART_Type* uart_base, unsigned int baud)
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
#if 0
    //设置波特率
    uart_base->UFCR &= ~(7<<7);
    uart_base->UFCR |= (5<<7); //对80Mhz进行1分频

    
    // uart_base->UBIR &= ~(0xffff);    //加了这个有问题
    uart_base->UBIR |= (71);
    // uart_base->UBMR &= ~(0xffff);    //加了这个有问题
    uart_base->UBMR |= (3124);
#endif
    uart_setbaudrate(uart_base,baud,80000000); //NXP官方写的设置波特率的函数
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


/*从串口发一个字符*/
void putc(unsigned char c)
{
    while((((UART1->USR2)>>3)&0X1)==0); //等待之前的数据发送完成
    UART1->UTXD = c;
}

/*从串口接收一个字符*/
unsigned char getc(void)
{
    while((((UART1->USR2)>>0)&0X1)==0); //等待有数据接收到
    return UART1->URXD;
}

/*从串口发送字符串*/
void puts(char* s)
{
    char *p=s;
    while(*p)
    {
        putc(*p);
        p++;
        led_on();

    }
    
}

/*使用printf函数*/

/*设置波特率*/
void uart_setbaudrate(UART_Type *base, unsigned int baudrate, unsigned int srcclock_hz)
{
    uint32_t numerator = 0u;		//分子
    uint32_t denominator = 0U;		//分母
    uint32_t divisor = 0U;
    uint32_t refFreqDiv = 0U;
    uint32_t divider = 1U;
    uint64_t baudDiff = 0U;
    uint64_t tempNumerator = 0U;
    uint32_t tempDenominator = 0u;

    /* get the approximately maximum divisor */
    numerator = srcclock_hz;
    denominator = baudrate << 4;
    divisor = 1;

    while (denominator != 0)
    {
        divisor = denominator;
        denominator = numerator % denominator;
        numerator = divisor;
    }

    numerator = srcclock_hz / divisor;
    denominator = (baudrate << 4) / divisor;

    /* numerator ranges from 1 ~ 7 * 64k */
    /* denominator ranges from 1 ~ 64k */
    if ((numerator > (UART_UBIR_INC_MASK * 7)) || (denominator > UART_UBIR_INC_MASK))
    {
        uint32_t m = (numerator - 1) / (UART_UBIR_INC_MASK * 7) + 1;
        uint32_t n = (denominator - 1) / UART_UBIR_INC_MASK + 1;
        uint32_t max = m > n ? m : n;
        numerator /= max;
        denominator /= max;
        if (0 == numerator)
        {
            numerator = 1;
        }
        if (0 == denominator)
        {
            denominator = 1;
        }
    }
    divider = (numerator - 1) / UART_UBIR_INC_MASK + 1;

    switch (divider)
    {
        case 1:
            refFreqDiv = 0x05;
            break;
        case 2:
            refFreqDiv = 0x04;
            break;
        case 3:
            refFreqDiv = 0x03;
            break;
        case 4:
            refFreqDiv = 0x02;
            break;
        case 5:
            refFreqDiv = 0x01;
            break;
        case 6:
            refFreqDiv = 0x00;
            break;
        case 7:
            refFreqDiv = 0x06;
            break;
        default:
            refFreqDiv = 0x05;
            break;
    }
    /* Compare the difference between baudRate_Bps and calculated baud rate.
     * Baud Rate = Ref Freq / (16 * (UBMR + 1)/(UBIR+1)).
     * baudDiff = (srcClock_Hz/divider)/( 16 * ((numerator / divider)/ denominator).
     */
    tempNumerator = srcclock_hz;
    tempDenominator = (numerator << 4);
    divisor = 1;
    /* get the approximately maximum divisor */
    while (tempDenominator != 0)
    {
        divisor = tempDenominator;
        tempDenominator = tempNumerator % tempDenominator;
        tempNumerator = divisor;
    }
    tempNumerator = srcclock_hz / divisor;
    tempDenominator = (numerator << 4) / divisor;
    baudDiff = (tempNumerator * denominator) / tempDenominator;
    baudDiff = (baudDiff >= baudrate) ? (baudDiff - baudrate) : (baudrate - baudDiff);

    if (baudDiff < (baudrate / 100) * 3)
    {
        base->UFCR &= ~UART_UFCR_RFDIV_MASK;
        base->UFCR |= UART_UFCR_RFDIV(refFreqDiv);
        base->UBIR = UART_UBIR_INC(denominator - 1); //要先写UBIR寄存器，然后在写UBMR寄存器，3592页 
        base->UBMR = UART_UBMR_MOD(numerator / divider - 1);
    }
}

/*链接第三方库的时候需要*/
int raise(void)
{
	return 0;
}
