# LCD实验

## 一、LCD驱动程序

### 1.1 裸机LCD开发流程
1. 初始化IMX6ULL的lcd控制器，初始化参数HSYNC、HSPW等等
2. 初始化lcd像素时钟
3. 设置RGBLCD显存
4. 应用程序直接通过显存来控制lcd

### 1.2 Linux LCD开发核心概念

#### （1）Framebuffer（帧缓冲）
Linux内核中用于抽象图形硬件的接口，通过 /dev/fbX 设备文件访问。NXP 官方的 Linux 内核默认已经开启了 LCD 驱动，因此我们是可以看到/dev/fb0 这样一个设备，如下：
```bash
/ # ls /dev/fb*
/dev/fb0
```
上面的`/dev/fb0`就是lcd的设备文件，且该设备是一个字符设备。该设备的操作函数集定义在`drivers/video/fbdev/core/fbmem.c`中。



#### （2）lcd控制器


#### （3）屏幕时序参数



### 1.3 驱动开发步骤

### （1）设备树配置
==**不同分辨率的 LCD 屏幕其 eLCDIF 控制器驱动代码都是一样的，只需要修改好对应的屏幕参数即可。**== 因此我们本章实验的主要工作就是修改设备树， NXP 官方的设备树已经添加了 LCD 设备节点，只是此节点的 LCD 屏幕信息是针对 NXP 官方 EVK 开发板所使用的 4.3 寸 480*272 编写的，我们需要将其改为我们所使用的屏幕参数。

重点需要关注的几点：
- LCD控制器的IO配置
- 屏幕节点的参数修改为我们使用的屏幕参数
- 背光节点的IO修改为我们使用的IO

```c#

/*imx6ull.dtsi文件中lcdif节点内容*/
lcdif: lcdif@021c8000 {
				compatible = "fsl,imx6ul-lcdif", "fsl,imx28-lcdif";     
				reg = <0x021c8000 0x4000>;
				interrupts = <GIC_SPI 5 IRQ_TYPE_LEVEL_HIGH>;
				clocks = <&clks IMX6UL_CLK_LCDIF_PIX>,
					 <&clks IMX6UL_CLK_LCDIF_APB>,
					 <&clks IMX6UL_CLK_DUMMY>;
				clock-names = "pix", "axi", "disp_axi";
				status = "disabled";
			};


//pinctl部分
&iomux{
    pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_hog_1>;
	imx6ul-evk {
        .......
        /*正点原子imx6ull的开发板的lcd接口屏幕上接了3个模拟开关，为了防止模拟开关影响网络，这里需要降低
         *lcd接口的驱动能力，将下面的0x79都修改为0x49
        */
        pinctrl_lcdif_dat: lcdifdatgrp { 
			fsl,pins = <
				MX6UL_PAD_LCD_DATA00__LCDIF_DATA00  0x79
				MX6UL_PAD_LCD_DATA01__LCDIF_DATA01  0x79
				MX6UL_PAD_LCD_DATA02__LCDIF_DATA02  0x79
				MX6UL_PAD_LCD_DATA03__LCDIF_DATA03  0x79
				MX6UL_PAD_LCD_DATA04__LCDIF_DATA04  0x79
				MX6UL_PAD_LCD_DATA05__LCDIF_DATA05  0x79
				MX6UL_PAD_LCD_DATA06__LCDIF_DATA06  0x79
				MX6UL_PAD_LCD_DATA07__LCDIF_DATA07  0x79
				MX6UL_PAD_LCD_DATA08__LCDIF_DATA08  0x79
				MX6UL_PAD_LCD_DATA09__LCDIF_DATA09  0x79
				MX6UL_PAD_LCD_DATA10__LCDIF_DATA10  0x79
				MX6UL_PAD_LCD_DATA11__LCDIF_DATA11  0x79
				MX6UL_PAD_LCD_DATA12__LCDIF_DATA12  0x79
				MX6UL_PAD_LCD_DATA13__LCDIF_DATA13  0x79
				MX6UL_PAD_LCD_DATA14__LCDIF_DATA14  0x79
				MX6UL_PAD_LCD_DATA15__LCDIF_DATA15  0x79
				MX6UL_PAD_LCD_DATA16__LCDIF_DATA16  0x79
				MX6UL_PAD_LCD_DATA17__LCDIF_DATA17  0x79
				MX6UL_PAD_LCD_DATA18__LCDIF_DATA18  0x79
				MX6UL_PAD_LCD_DATA19__LCDIF_DATA19  0x79
				MX6UL_PAD_LCD_DATA20__LCDIF_DATA20  0x79
				MX6UL_PAD_LCD_DATA21__LCDIF_DATA21  0x79
				MX6UL_PAD_LCD_DATA22__LCDIF_DATA22  0x79
				MX6UL_PAD_LCD_DATA23__LCDIF_DATA23  0x79
			>;
		};

		pinctrl_lcdif_ctrl: lcdifctrlgrp {
			fsl,pins = <
				MX6UL_PAD_LCD_CLK__LCDIF_CLK	    0x79
				MX6UL_PAD_LCD_ENABLE__LCDIF_ENABLE  0x79
				MX6UL_PAD_LCD_HSYNC__LCDIF_HSYNC    0x79
				MX6UL_PAD_LCD_VSYNC__LCDIF_VSYNC    0x79
			>;
		};
    }
}




/*imx6ull-alientek-emmc.dts文件中lcdif节点内容*/
&lcdif {
	pinctrl-names = "default";
    /*正点原子的开发板没有使用屏幕reset接口，&pinctrl_lcdif_reset可以删掉*/
	pinctrl-0 = <&pinctrl_lcdif_dat
		     &pinctrl_lcdif_ctrl
		     &pinctrl_lcdif_reset>;
	display = <&display0>;
	status = "okay";

	display0: display {
		bits-per-pixel = <16>;      //一个像素占用的内存，我们使用的RGB888，所有这里要改为24bit
		bus-width = <24>;           

		display-timings {
			native-mode = <&timing0>;
			timing0: timing0 {
            /*下面这些参数改为对应屏幕的参数*/
			clock-frequency = <9200000>;        //像素时钟，单位hz
			hactive = <480>;                    //x轴像素个数
			vactive = <272>;                    //y轴像素个数
			hfront-porch = <8>;                 //hfp参数
			hback-porch = <4>;                  //hbp参数
			hsync-len = <41>;                   //hspw参数
			vback-porch = <2>;                  //vbp参数
			vfront-porch = <4>;                 //vfp参数
			vsync-len = <10>;                   //vspw参数

			hsync-active = <0>;
			vsync-active = <0>;
			de-active = <1>;
			pixelclk-active = <0>;
			};
		};
	};
};


//正点原子屏幕的背光使用和官方开发板一致，不需要修改背光节点

```

### （2）驱动程序流程
Linux 内核将所有的 Framebuffer 抽象为一个叫做 fb_info 的结构体， fb_info 结构体包含了 Framebuffer 设备的完整属性和操作集合，因此每一个 Framebuffer 设备都必须有一个 fb_info。换言之就是， LCD 的驱动就是构建 fb_info，并且向系统注册 fb_info的过程。 fb_info 结构体定义在 `include/linux/fb.h` 文件里面
```c
struct fb_info {
	atomic_t count;
	int node;
	int flags;
	struct mutex lock;		/* Lock for open/release/ioctl funcs */
	struct mutex mm_lock;		/* Lock for fb_mmap and smem_* fields */
	struct fb_var_screeninfo var;	/* 重点，当前可变参数 */   
	struct fb_fix_screeninfo fix;	/* 重点，当前固定参数 */
    ......
    struct fb_ops *fbops;       /*重点， 帧缓冲操作函数集*/
    ......
    char __iomem *screen_base;	/*重点，显存的虚拟内存基地址*/
    unsigned long screen_size;	/*重点，显存大小*/ 
    void *pseudo_palette;       /*重点，伪 16 位调色板 */

    .......
```

在`drivers/video/fbdev/mxsfb.c`中nxp官方的屏幕驱动文件中，platform的probe函数中主要做了以下几个步骤：

1. 申请`fb_info`
2. 初始化`fb_info`的各个成员变量
3. 初始化eLCDIF 控制器
4. 使用 register_framebuffer 函数向 Linux 内核注册初始化好的 fb_info。register_framebuffer函数原型如下：
   ```c
   int register_framebuffer(struct fb_info *fb_info)
   ```



## 二、LCD应用程序
Linux应用程序也是通过操作显存来控制lcd的，在linux中一般使用QT、LVGL等UI库来编写LCD的UI。