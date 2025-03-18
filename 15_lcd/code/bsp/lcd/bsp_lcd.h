#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include "imx6ul.h"

#define ATK4342		0X4342	/* 4.3寸480*272 	*/
#define ATK4384		0X4384	/* 4.3寸800*480 	*/
#define ATK7084		0X7084	/* 7寸800*480 		*/
#define ATK7016		0X7016	/* 7寸1024*600 		*/
#define ATK1018		0X1018	/* 10.1寸1280*800 	*/
#define ATKVGA		0xff00 /* VGA */

/*lcd屏幕信息结构体，为了兼容同的屏幕*/
struct tftlcd_typedef{
    unsigned short height;  //屏幕高度
    unsigned short weight;  //屏幕宽度
    unsigned char pixel_size;   //每个像素的大小
    unsigned short vspw;
    unsigned short vbp;
    unsigned short vfp;
    unsigned short hspw;
    unsigned short hbp;
    unsigned short hfp;
    unsigned int frameBuffer;   //屏幕显存起始地址
    unsigned int forecolor;     //前景色
    unsigned int backcolor;     //背景色
};



void lcd_init();
unsigned short lcd_read_panelid(void);
void lcd_io_init();
void lcd_reset();
void lcd_noreset();
void lcd_enable();

#endif
