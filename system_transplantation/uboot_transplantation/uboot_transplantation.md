# uboot移植实验



## nxp官方uboot编译与测试



- 编译nxp官方的uboot

- 一个开发板要运行uboot,必须要有**DDR(DRAM)**,**串口**,**SD**,**EMMC**,**NAND**

测试结果

- **DDR(DRAM)**,**串口**,**SD**,**EMMC**,**NAND**正常
- 屏幕和网卡驱动不正常

## 在uboot中添加正点原子开发板的uboot



### 1.添加板子配置文件

参考nxp官方的配置文件，拷贝nxp官方的配置文件`cp configs/mx6ull_14x14_evk_emmc_defconfig configs/mx6ull_alientek_emmc_defconfig`

defconfig文件中修改如下内容：
```
//原
CONFIG_SYS_EXTRA_OPTIONS="IMX_CONFIG=board/freescale/mx6ullevk/imximage.cfg,MX6ULL_EVK_EMMC_REWORK"
CONFIG_TARGET_MX6ULL_14X14_EVK=y
//现
CONFIG_SYS_EXTRA_OPTIONS="IMX_CONFIG=board/freescale/mx6ull_alientek/imximage.cfg,MX6ULL_EVK_EMMC_REWORK"
CONFIG_TARGET_MX6ULL_ALIENTEK=y   
```



### 2.添加板子的头文件

不同的板子，由不同的配置信息。一般在一个头文件中配置，例如nxp的mx6ull_evk开发板的`./include/configs/mx6ullevk.h`

同样，拷贝这个文件命名为mx6ull_alientek.h，修改开头的条件编译

```
#ifndef __MX6ULL_ALIENTEK_EMMC_CONFIG_H
#define __MX6ULL_ALIENTEK_EMMC_CONFIG_H
```

### 3.添加开发板的板级文件夹

拷贝`./board/freescale/mx6ullevk`到`./board/freescale/mx6ull_alientek`

把其中的.c文件改名，然后修改makefile中的目标，imximage.cfg中的34行PLUGIN路径要改，修改MAINTAINERS中的路径，最后修改Kconfig文件

```
if TARGET_MX6ULL_ALIENTEK || TARGET_MX6ULL_9X9_EVK

config SYS_BOARD
	default "mx6ull_alientek"

config SYS_VENDOR
	default "freescale"

config SYS_CONFIG_NAME
	default "mx6ull_alientek"

endif
```


### 4.添加图形化配置文件


最后修改`arch/arm/cpu/armv7/mx6/Kconfig`文件，加入下面的内容
```
config TARGET_MX6ULL_ALIENTEK
	bool "Support mx6ull_alientek"
	select MX6ULL
	select DM
	select DM_THERMAL
	
source "board/freescale/mx6ull_alientek/Kconfig"
```

### 5.修改屏幕驱动

驱动文件一般在board/freescale/\*/\*.c（\*代表板子名字）和include/configs/\*.h（*代表板子名字）

一般修改 LCD 驱动重点注意以下几点：
①、 LCD 所使用的 GPIO，查看 uboot 中 LCD 的 IO 配置是否正确。
②、 LCD 背光引脚 GPIO 的配置。
③、 LCD 配置参数是否正确。

IO不用改，2块板子的io设置一样

```
struct display_info_t const displays[] = {{
	.bus = MX6UL_LCDIF1_BASE_ADDR,
	.addr = 0,
	.pixfmt = 24,
	.detect = NULL,
	.enable	= do_enable_parallel_lcd,
	.mode	= {
		.name			= "TFT7016",
		.xres           = 1024,
		.yres           = 600,
		.pixclock       = 19531,	//打印一个像素点需要的ps数，1/(帧率对应的时钟)*10^12
		.left_margin    = 140,
		.right_margin   = 160,
		.upper_margin   = 20,
		.lower_margin   = 12,
		.hsync_len      = 20,
		.vsync_len      = 3,
		.sync           = 0,
		.vmode          = FB_VMODE_NONINTERLACED
} } };

//在其他文件
struct display_info_t {
	int	bus;
	int	addr;
	int	pixfmt;
	int	(*detect)(struct display_info_t const *dev);
	void	(*enable)(struct display_info_t const *dev);
	struct	fb_videomode mode;
};
//在其他文件
struct fb_videomode {
	const char *name;	/* optional */
	u32 refresh;		/* optional */
	u32 xres;
	u32 yres;
	u32 pixclock;
	u32 left_margin;
	u32 right_margin;
	u32 upper_margin;
	u32 lower_margin;
	u32 hsync_len;
	u32 vsync_len;
	u32 sync;
	u32 vmode;
	u32 flag;
};
结构体 fb_videomode 里面的成员变量为 LCD 的参数，这些成员变量函数如下：
name： LCD 名字，要和环境变量中的 panel 相等。
xres、 yres： LCD X 轴和 Y 轴像素数量。
pixclock：像素时钟，每个像素时钟周期的长度，单位为皮秒。
left_margin： HBP，水平同步后肩。
right_margin： HFP，水平同步前肩。
upper_margin： VBP，垂直同步后肩。
lower_margin： VFP，垂直同步前肩。
hsync_len： HSPW，行同步脉宽。
vsync_len： VSPW，垂直同步脉宽。
vmode： 大多数使用 FB_VMODE_NONINTERLACED，也就是不使用隔行扫描
```

在.h文件中有板子的环境变量设置，要修改其中的panel变量`panel=TFT7016`

### 6.修改网络驱动