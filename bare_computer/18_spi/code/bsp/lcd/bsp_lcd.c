#include "bsp_lcd.h"
#include "bsp_gpio.h"
#include "stdio.h"
#include "bsp_delay.h"

struct tftlcd_typedef tftlcd_dev;

/*初始化LCD*/
void lcd_init()
{
    //读取屏幕ID
    unsigned short lcd_id = lcd_read_panelid();
    printf("LCD_ID:%#x\r\n",lcd_id);

    //初始化IO
    lcd_io_init();
    lcd_reset();
    delay_ms(10);
	lcd_noreset();

	//对不同的屏幕进行初始化
	lcd_info_init(lcd_id);
	
	lcdif_init();

	//使能lcdif
	lcd_enable();
	delay_ms(10);
	lcd_clear(LCD_WHITE);

}

/*lcdif接口配置*/
void lcdif_init()
{
	LCDIF->CTRL = 0;
	LCDIF->CTRL |= (1<<5)|(3<<8)|(3<<10)|(1<<17)|(1<<19);	

	LCDIF->CTRL1 = 0;
	LCDIF->CTRL1 |= (7<<16);

	LCDIF->TRANSFER_COUNT = 0;
	LCDIF->TRANSFER_COUNT |= (tftlcd_dev.width<<0)|(tftlcd_dev.height<<16);

	LCDIF->VDCTRL0 = 0;
	LCDIF->VDCTRL0 |= (tftlcd_dev.vspw<<0)|(1<<20)|(1<<21)|(1<<24)|(1<<28);

	LCDIF->VDCTRL1 = 0;
	LCDIF->VDCTRL1 |= (tftlcd_dev.vbp + tftlcd_dev.vfp + tftlcd_dev.vspw + tftlcd_dev.height);

	LCDIF->VDCTRL2 = 0;
	LCDIF->VDCTRL2 |= (tftlcd_dev.hspw + tftlcd_dev.hfp + tftlcd_dev.width + tftlcd_dev.hbp)|
						(tftlcd_dev.hspw<<18);

	LCDIF->VDCTRL3 = 0;
	LCDIF->VDCTRL3 |= (tftlcd_dev.vbp + tftlcd_dev.vspw)|((tftlcd_dev.hbp + tftlcd_dev.hspw)<<16);

	LCDIF->VDCTRL4 = 0;
	LCDIF->VDCTRL4 |= (tftlcd_dev.width)|(1<<18);

	LCDIF->CUR_BUF = 0;
	LCDIF->CUR_BUF |= tftlcd_dev.frameBuffer;

	LCDIF->NEXT_BUF = 0;
	LCDIF->NEXT_BUF |= tftlcd_dev.frameBuffer;
}

/*根据不同的屏幕ID初始化屏幕信息*/
void lcd_info_init(unsigned short lcd_id)
{
	//根据不同的屏幕ID初始化屏幕信息
	tftlcd_dev.pixel_size = 4;	//每个像素4个字节，ARGB8888
	tftlcd_dev.frameBuffer = LCD_FRAMEBUF_ADDR;
	tftlcd_dev.forecolor = LCD_WHITE;	//前景色白
	tftlcd_dev.backcolor = LCD_BLACK;	//背景黑

	switch (lcd_id)
	{
	case ATK1018:
		tftlcd_dev.height = 800;	
		tftlcd_dev.width = 1280;
		tftlcd_dev.vspw = 3;
		tftlcd_dev.vbp = 10;
		tftlcd_dev.vfp = 10;
		tftlcd_dev.hspw = 10;
		tftlcd_dev.hbp = 80;
		tftlcd_dev.hfp = 70;
		lcd_clk_init(35, 3, 5);	/* 初始化LCD时钟 56MHz */
		break;
	case ATK4342:
		tftlcd_dev.height = 272;	
		tftlcd_dev.width = 480;
		tftlcd_dev.vspw = 1;
		tftlcd_dev.vbp = 8;
		tftlcd_dev.vfp = 8;
		tftlcd_dev.hspw = 1;
		tftlcd_dev.hbp = 40;
		tftlcd_dev.hfp = 5; 
		lcd_clk_init(27, 8, 8);	/* 初始化LCD时钟 10.1MHz */
		break;
	case ATK4384:
		tftlcd_dev.height = 480;	
		tftlcd_dev.width = 800;
		tftlcd_dev.vspw = 3;
		tftlcd_dev.vbp = 32;
		tftlcd_dev.vfp = 13;
		tftlcd_dev.hspw = 48;
		tftlcd_dev.hbp = 88;
		tftlcd_dev.hfp = 40;
		lcd_clk_init(42, 4, 8);	/* 初始化LCD时钟 31.5MHz */
		break;
	case ATK7016:
		tftlcd_dev.height = 600;	
		tftlcd_dev.width = 1024;
		tftlcd_dev.vspw = 3;
		tftlcd_dev.vbp = 20;
		tftlcd_dev.vfp = 12;
		tftlcd_dev.hspw = 20;
		tftlcd_dev.hbp = 140;
		tftlcd_dev.hfp = 160;
		lcd_clk_init(32,3,5);	//设置51.2Mhz的时钟，60hz刷新率
		break;
	case ATK7084:
		tftlcd_dev.height = 480;	
		tftlcd_dev.width = 800;
		tftlcd_dev.vspw = 1;
		tftlcd_dev.vbp = 23;
		tftlcd_dev.vfp = 22;
		tftlcd_dev.hspw = 1;
		tftlcd_dev.hbp = 46;
		tftlcd_dev.hfp = 210;
		lcd_clk_init(30, 3, 7);	/* 初始化LCD时钟 34.2MHz */
		break;
	case ATKVGA:
		tftlcd_dev.height = 768;	
		tftlcd_dev.width = 1366;
		tftlcd_dev.vspw = 3;
		tftlcd_dev.vbp = 24;
		tftlcd_dev.vfp = 3;
		tftlcd_dev.hspw = 143;
		tftlcd_dev.hbp = 213;
		tftlcd_dev.hfp = 70;
		lcd_clk_init(32, 3, 3);	/* 初始化LCD时钟 85MHz */
		break;
	default:	//默认1024*600
		tftlcd_dev.height = 600;	
		tftlcd_dev.width = 1024;
		tftlcd_dev.vspw = 3;
		tftlcd_dev.vbp = 20;
		tftlcd_dev.vfp = 12;
		tftlcd_dev.hspw = 20;
		tftlcd_dev.hbp = 140;
		tftlcd_dev.hfp = 160;
		lcd_clk_init(32,3,5);	//设置51.2Mhz的时钟，60hz
		break;
	}
}


/*时钟设置
时钟为24*loopDiv/prediv/div Mhz
loopDiv 设置范围为27-54
prediv 设置范围为1-8
div 设置范围为1-5
*/
void lcd_clk_init(unsigned char loopDiv,unsigned char prediv,unsigned char div)
{
	CCM_ANALOG->PLL_VIDEO_DENOM = 0;
	CCM_ANALOG->PLL_VIDEO_NUM = 0;

	CCM_ANALOG->PLL_VIDEO = ((1<<13)|(2<<19)|(unsigned int)loopDiv);

	CCM_ANALOG->MISC2 = 0;

	/* LCD时钟源来源与PLL5，也就是VIDEO           PLL  */
	CCM->CSCDR2 &= ~(7 << 15);  	
	CCM->CSCDR2 |= (2 << 15);			/* 设置LCDIF_PRE_CLK使用PLL5 */

	/* 设置LCDIF_PRE分频 */
	CCM->CSCDR2 &= ~(7 << 12);		
	CCM->CSCDR2 |= ((unsigned int)prediv - 1) << 12;	/* 设置分频  */

	/* 设置LCDIF分频 */
	CCM->CBCMR &= ~(7 << 23);					
	CCM->CBCMR |= ((unsigned int)div - 1) << 23;				

	/* 设置LCD时钟源为LCDIF_PRE时钟 */
	CCM->CSCDR2 &= ~(7 << 9);					/* 清除原来的设置		 	*/
	CCM->CSCDR2 |= (0 << 9);					/* LCDIF_PRE时钟源选择LCDIF_PRE时钟 */

}


/*lcd复位*/
void lcd_reset()
{
    LCDIF->CTRL = (1<<31);
}

/*lcd停止复位*/
void lcd_noreset()
{
    LCDIF->CTRL = 0;
}

/*使能LCD控制器*/
void lcd_enable()
{
    LCDIF->CTRL |= (1<<0);
}



/*初始化LCD的IO*/
void lcd_io_init()
{
    //设置LCD的数据io复用

    IOMUXC_SetPinMux(IOMUXC_LCD_DATA00_LCDIF_DATA00,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA01_LCDIF_DATA01,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA02_LCDIF_DATA02,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA03_LCDIF_DATA03,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA04_LCDIF_DATA04,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA05_LCDIF_DATA05,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA06_LCDIF_DATA06,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_LCDIF_DATA07,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA08_LCDIF_DATA08,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA09_LCDIF_DATA09,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA10_LCDIF_DATA10,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA11_LCDIF_DATA11,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA12_LCDIF_DATA12,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA13_LCDIF_DATA13,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA14_LCDIF_DATA14,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_LCDIF_DATA15,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA16_LCDIF_DATA16,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA17_LCDIF_DATA17,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA18_LCDIF_DATA18,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA19_LCDIF_DATA19,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA20_LCDIF_DATA20,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA21_LCDIF_DATA21,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA22_LCDIF_DATA22,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_LCDIF_DATA23,0);

    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA00_LCDIF_DATA00,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA01_LCDIF_DATA01,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA02_LCDIF_DATA02,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA03_LCDIF_DATA03,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA04_LCDIF_DATA04,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA05_LCDIF_DATA05,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA06_LCDIF_DATA06,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_LCDIF_DATA07,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA08_LCDIF_DATA08,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA09_LCDIF_DATA09,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA10_LCDIF_DATA10,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA11_LCDIF_DATA11,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA12_LCDIF_DATA12,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA13_LCDIF_DATA13,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA14_LCDIF_DATA14,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_LCDIF_DATA15,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA16_LCDIF_DATA16,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA17_LCDIF_DATA17,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA18_LCDIF_DATA18,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA19_LCDIF_DATA19,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA20_LCDIF_DATA20,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA21_LCDIF_DATA21,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA22_LCDIF_DATA22,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_LCDIF_DATA23,0xB9);



	IOMUXC_SetPinMux(IOMUXC_LCD_CLK_LCDIF_CLK,0);	
	IOMUXC_SetPinMux(IOMUXC_LCD_ENABLE_LCDIF_ENABLE,0);	
	IOMUXC_SetPinMux(IOMUXC_LCD_HSYNC_LCDIF_HSYNC,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_LCDIF_VSYNC,0);


    IOMUXC_SetPinConfig(IOMUXC_LCD_CLK_LCDIF_CLK,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_ENABLE_LCDIF_ENABLE,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_HSYNC_LCDIF_HSYNC,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_LCDIF_VSYNC,0xB9);

	IOMUXC_SetPinMux(IOMUXC_GPIO1_IO08_GPIO1_IO08,0);			/* 背光BL引脚      */
	IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO08_GPIO1_IO08,0x10b0);

    //设置GPIO8为输出,点亮背光
    _gpio_pin_config_t pin_config;
    pin_config.direction = kGPIO_Digitaloutput;
    pin_config.outputLogic = 1U;
    pin_config.interruptMode = kgpio_Nointmode;
    gpio_init(GPIO1,8,&pin_config);	


}





/*
 * 读取屏幕ID，
 * 描述：LCD_DATA23=R7(M0);LCD_DATA15=G7(M1);LCD_DATA07=B7(M2);
 * 		M2:M1:M0
 *		0 :0 :0	//4.3寸480*272 RGB屏,ID=0X4342
 *		0 :0 :1	//7寸800*480 RGB屏,ID=0X7084
 *	 	0 :1 :0	//7寸1024*600 RGB屏,ID=0X7016
 *  	1 :0 :1	//10.1寸1280*800,RGB屏,ID=0X1018
 *		1 :0 :0	//4.3寸800*480 RGB屏,ID=0X4384
 * @param 		: 无
 * @return 		: 屏幕ID
 */
unsigned short lcd_read_panelid(void)
{
	unsigned char idx = 0;

	/* 配置屏幕ID信号线 */
	IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_GPIO3_IO03, 0);
	IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_GPIO3_IO03, 0X10B0);

	/* 打开模拟开关 */
	_gpio_pin_config_t idio_config;
	idio_config.direction = kGPIO_Digitaloutput;
	idio_config.outputLogic = 1;
	gpio_init(GPIO3, 3, &idio_config);

	/* 读取ID值，设置G7 B7 R7为输入 */
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_GPIO3_IO12, 0);		/* B7(M2) */
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_GPIO3_IO20, 0);		/* G7(M1) */
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_GPIO3_IO28, 0);		/* R7(M0) */

	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_GPIO3_IO12, 0xF080);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_GPIO3_IO20, 0xF080);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_GPIO3_IO28, 0xF080);  

	idio_config.direction = kGPIO_Digitalinput;
	gpio_init(GPIO3, 12, &idio_config);
	gpio_init(GPIO3, 20, &idio_config);
	gpio_init(GPIO3, 28, &idio_config);

	idx = (unsigned char)gpio_pin_read(GPIO3, 28); 	/* 读取M0 */
	idx |= (unsigned char)gpio_pin_read(GPIO3, 20) << 1;	/* 读取M1 */
	idx |= (unsigned char)gpio_pin_read(GPIO3, 12) << 2;	/* 读取M2 */

	if(idx==0)return ATK4342;		//4.3寸屏,480*272分辨率
	else if(idx==1)return ATK7084;	//7寸屏,800*480分辨率
	else if(idx==2)return ATK7016;	//7寸屏,1024*600分辨率
	else if(idx==4)return ATK4384;	//4寸屏,800*480分辨率
	else if(idx==5)return ATK1018;	//10.1寸屏,1280*800分辨率		
	else if(idx==7)return ATKVGA;   //VGA模块，1366*768分辨率
	else return 0;

}


/*画点函数*/
inline void lcd_drawPoint(unsigned short x, unsigned short y, unsigned int color)
{
	//计算x，y坐标对应的数组位置
	*((unsigned int *)((unsigned int)tftlcd_dev.frameBuffer + tftlcd_dev.pixel_size *
									(tftlcd_dev.width * y + x))) = color;


}

/*读点函数*/
inline unsigned int lcd_readPoint(unsigned short x, unsigned short y)
{
	return *((unsigned int *)((unsigned int)tftlcd_dev.frameBuffer + tftlcd_dev.pixel_size *
	(tftlcd_dev.width * y + x)));
}


/*清屏函数*/
void lcd_clear(unsigned int color)
{
	unsigned int num;
	unsigned int i=0;
	unsigned int* start_addr = (unsigned int*)tftlcd_dev.frameBuffer;
	num = (unsigned int)(tftlcd_dev.width*tftlcd_dev.height);
	for(i=0;i<num;i++)
	{
		start_addr[i] = color;
	}

}


void lcd_fill(unsigned    short x0, unsigned short y0, 
	unsigned short x1, unsigned short y1, unsigned int color)
{ 
unsigned short x, y;

if(x0 < 0) x0 = 0;
if(y0 < 0) y0 = 0;
if(x1 >= tftlcd_dev.width) x1 = tftlcd_dev.width - 1;
if(y1 >= tftlcd_dev.height) y1 = tftlcd_dev.height - 1;

for(y = y0; y <= y1; y++)
{
for(x = x0; x <= x1; x++)
lcd_drawPoint(x, y, color);
}
}



