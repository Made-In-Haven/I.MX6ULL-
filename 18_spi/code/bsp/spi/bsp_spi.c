#include "bsp_spi.h"
#include "stdio.h"

/*初始化spi，时钟为7.5Mhz*/
void spi_init(ECSPI_Type *base)
{
    /*关闭spi*/
    base->CONREG = 0;


    /*设置时钟为7.5Mhz*/
    CCM->CCSR &= ~(1<<0);
    CCM->CSCDR2 &= ~(1<<18);
    CCM->CSCDR2 &= ~(0X3F<<19);



    base->CONREG |= ((2<<8)|(3<<12));

    base->CONREG |= (1<<3)|(1<<4)|(0x7<<20);
    
    base->CONFIGREG = 0;
    base->CONREG |= (1);
    base->PERIODREG = (0x2000);
}

unsigned char spich0_readwrite_byte(ECSPI_Type *base, unsigned char txdata)
{
    uint32_t spirxdata = 0;
    uint32_t spitxdata = txdata;

    /*选择通道0*/
    base->CONREG &= ~(0x3<<18);

    /*数据发送*/
    while((base->STATREG&(0x1))==0);    //等待TXFIFO为空
    // if(((base->STATREG>>7)&0x1)==1)     //传输完成标志位若为1
    // {
    //     base->STATREG |= (1<<7);    //传输完成标志位写1清除
    // }
    base->TXDATA = spitxdata;

    /*数据接收*/
    while((((base->STATREG)>>3)&(0x1))==0);   //等待数据到来
    spirxdata = base->RXDATA;
    return (unsigned char)spirxdata;
}