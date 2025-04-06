#include "bsp_clk.h"

void clk_enable(void)
{
	CCM->CCGR0 = 0XFFFFFFFF;
	CCM->CCGR1 = 0XFFFFFFFF;

	CCM->CCGR2 = 0XFFFFFFFF;
	CCM->CCGR3 = 0XFFFFFFFF;
	CCM->CCGR4 = 0XFFFFFFFF;
	CCM->CCGR5 = 0XFFFFFFFF;
	CCM->CCGR6 = 0XFFFFFFFF;
}

/*初始化主频时钟*/
void clk_init(void)
{
	/*初始化主频为528Mhz*/
	if((((CCM->CCSR)>>2)&(0X1))==0) // 当前PLL1时钟使用pll1_main_clk
	{
		
		CCM->CCSR &= ~(1<<8);//设置STEP_SEL选择24Mhz
		CCM->CCSR |= (0x1<<2);//切换至step_clk

	}
	//修改寄存器特定位,读-改-写
	uint32_t reg_value = 0;
	reg_value = CCM_ANALOG->PLL_ARM;
	reg_value &= ~(0x7f);
	reg_value |= ((66<<0) & 0X7F);
	CCM_ANALOG->PLL_ARM = reg_value;//设置88倍频，输出1056Mhz
	CCM_ANALOG->PLL_ARM |= (0x1<<13);//使能CCM_ANALOG->PLL_ARM

	CCM->CCSR &= ~(0x1<<2);//切换至pll1_main_clk  1056Mhz
	CCM->CACRR &= ~(0X1);//设置pll1到ARM_CLK_ROOT二分频，生成主频528Mhz


	//PLL2 System_pll固定为528M
	reg_value = 0;
	reg_value = CCM_ANALOG->PFD_528;
	reg_value &= ~((0x3f)|(0x3f<<8)|(0x3f<<16)|(0x3f<<24));
	reg_value |= ((32&0x3f)<<24);		//PLL2_PFD3=297
	reg_value |= (24<<16);				//PLL2_PFD2=396
	reg_value |= (16<<8);				//PLL2_PFD1=594
	reg_value |= (27<<0);				//PLL2_PFD0=352
	CCM_ANALOG->PFD_528 = reg_value;

	//PLL3 Usb1_pll固定为480M
	reg_value = 0;
	reg_value = CCM_ANALOG->PFD_480;
	reg_value &= ~((0x3f)|(0x3f<<8)|(0x3f<<16)|(0x3f<<24));
	reg_value |= (19<<24);		//PLL3_PFD3=454.7
	reg_value |= (17<<16);				//PLL3_PFD2=508.2
	reg_value |= (16<<8);				//PLL3_PFD1=540
	reg_value |= (12<<0);				//PLL3_PFD0=720
	CCM_ANALOG->PFD_480 = reg_value;

	//AHB_CLK_ROOT 132Mhz
	CCM->CBCMR &= ~(0X3<<18);
	CCM->CBCMR |= (1<<18);
	CCM->CBCDR &= ~(1<<25);
	while(CCM->CDHIPR & (1<<5));
#if 0
	CCM->CBCDR &= ~(7<<10);
	while(CCM->CDHIPR & (1<<1));//等待握手信号
	CCM->CBCDR |= (2<<10);
	while(CCM->CDHIPR & (1<<1));//等待握手信号
#endif

	
	//IPG_CLK_ROOT 66Mhz
	CCM->CBCDR &= ~(3<<8);
	CCM->CBCDR |= (1<<8);

	
	//PERCLK_CLK_ROOT 66Mhz
	CCM->CSCMR1 &= ~(1<<6);
	CCM->CSCMR1 &= ~(0X3F<<0);
	

}






