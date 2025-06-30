# Pinctl子系统和GPIO子系统



## Pinctl子系统

Pinctl的主要工作内容：

- 获取设备数中的PIN信息

- 根据获取的PIN信息设置复用功能

- 根据获取的PIN信息设置电气属性

对于使用者来说，只需要再设备数里设置好某个PIN的信息，其他工作均由Pinctl子系统完成。

在imx6ull.dtsi文件中有IOMUXC SNVS控制器的描述：

```c
iomuxc_snvs: iomuxc-snvs@02290000 {						//复用控制器
				compatible = "fsl,imx6ull-iomuxc-snvs";
				reg = <0x02290000 0x10000>;
			};
iomuxc: iomuxc@020e0000 {								//复用IO寄存器
				compatible = "fsl,imx6ul-iomuxc";
				reg = <0x020e0000 0x4000>;
			};
```

在imx6ull-alientek-emmc.dts文件中有iomuxc的追加内容：

```c
&iomuxc {
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_hog_1>;
	imx6ul-evk {
		pinctrl_hog_1: hoggrp-1 {
			fsl,pins = <
				MX6UL_PAD_UART1_RTS_B__GPIO1_IO19	0x17059 /* SD1 CD */
				MX6UL_PAD_GPIO1_IO05__USDHC1_VSELECT	0x17059 /* SD1 VSELECT */
				MX6UL_PAD_GPIO1_IO09__GPIO1_IO09        0x17059 /* SD1 RESET */
			>;
		};
	.......
}
```

以pinctrl_hog_1为例，MX6UL_PAD_UART1_RTS_B__GPIO1_IO19为引脚复用，在imx6ul-pinfunc.h或imx6ull-pinfunc.h中有定义：

```c
 /* The pin function ID is a tuple of
 * <mux_reg conf_reg input_reg mux_mode input_val>
 */
#define MX6UL_PAD_UART1_RTS_B__GPIO1_IO19              0x0090 0x031C 0x0000 0x5 0x0
```

- 0x0090是MX6UL_PAD_UART1_RTS_B复用寄存器相对于iomuxc基地址的偏移
- 0x031C是MX6UL_PAD_UART1_RTS_B电气属性配置寄存器相对于iomuxc基地址的偏移，设备树中的`MX6UL_PAD_UART1_RTS_B__GPIO1_IO19	0x17059 /* SD1 CD */`，其中的0x17059就是写入电气属性配置寄存器的值

- 0x0000偏移为0，表示MX6UL_PAD_UART1_RTS_B这个Pin没有输入引脚选择功能
- 0x5是MX6UL_PAD_UART1_RTS_B复用寄存器的值，表示复用为GPIO1_IO19
- 0x0，写给input reg寄存器的值





## GPIO子系统

==当我们把一个Pin复用为GPIO时就会用到GPIO子系统==

```c
&iomuxc {
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_hog_1>;
	imx6ul-evk {
		pinctrl_hog_1: hoggrp-1 {
			fsl,pins = <
				MX6UL_PAD_UART1_RTS_B__GPIO1_IO19	0x17059 /*检测SD卡是否插入*/
				MX6UL_PAD_GPIO1_IO05__USDHC1_VSELECT	0x17059 /* SD1 VSELECT */
				MX6UL_PAD_GPIO1_IO09__GPIO1_IO09        0x17059 /* SD1 RESET */
			>;
		};
	.......
}
```

以**MX6UL_PAD_GPIO1_IO09__GPIO1_IO19**复用为例，SD卡控制器**usdhc1**的描述如下

```c
&usdhc1 {
	pinctrl-names = "default", "state_100mhz", "state_200mhz";
	pinctrl-0 = <&pinctrl_usdhc1>;
	pinctrl-1 = <&pinctrl_usdhc1_100mhz>;
	pinctrl-2 = <&pinctrl_usdhc1_200mhz>;
	cd-gpios = <&gpio1 19 GPIO_ACTIVE_LOW>;		//重点
	keep-power-in-suspend;
	enable-sdio-wakeup;
	vmmc-supply = <&reg_sd1_vmmc>;
	status = "okay";
};
```

pinctrl-0/1/2对应了3种不同的电气属性配置

**cd-gpios属性表示使用的是GPIO1_IO19，低电平有效**

之后我们就可以使用of函数**of_get_named_gpio**来读取该属性。





## 实验步骤



### 修改设备树

#### 在根节点下添加led设备节点，在设备节点下添加GPIO属性和pinctl

```c
led	{
		compatible = "alientek,dtsled";
		pinctrl-names = "default"
		pinctrl-0 = <&pinctrl_led>;				//指定pinctl
		#address-cells = <1>;
		#size-cells = <1>;
		status = "okay";
		led-gpio = <&gpio1 3 GPIO_ACTIVE_LOW>;	//GPIO属性	
	};
```



#### 在iomux节点下的imx6ul下添加led的pinctrl_led

```c
pinctrl_led:ledgrp {
			fsl,pins = <
			MX6UL_PAD_GPIO1_IO03__GPIO1_IO03	0X10B0
			>;
		};
```











### 驱动中对GPIO的操作

**首先要手动在设备树中查询这个IO是否被其他外设占用，这一点非常重要**

1. 首先，使用of_find_node_by_path获取GPIO设备节点
2. 获取GPIO编号，**of_get_named_gpio**
3. 请求对应编号的GPIO，**gpio_request**
4. 设置GPIO，有相应的API函数**gpio_direction_input**、**gpio_direction_input**、**gpio_get_value**、**gpio_set_value**
5. 释放对应编号的**gpio_free**





**of_get_named_gpio**获取 GPIO 编号，因为 Linux 内核中关于 GPIO 的 API 函数都要使用 GPIO 编号，此函数会将设备树中类似<&gpio5 7 GPIO_ACTIVE_LOW>的属性信息转换为对应的 GPIO 编号，此函数在驱动中使用很频繁！函数原型如下：

```c
/*
np：设备节点
propname：包含要获取 GPIO 信息的属性名
index： GPIO 索引，因为一个属性里面可能包含多个GPIO，此参数指定要获取哪个GPIO的编号，如果只有一个 GPIO 信息的话此参数为 0。
*/

int of_get_named_gpio(struct device_node *np, const char *propname, int index)
    
//例子
ov5640: ov5640@3c {
		compatible = "ovti,ov5640";
		reg = <0x3c>;
		pinctrl-names = "default";
		pinctrl-0 = <&pinctrl_csi1>;
		clocks = <&clks IMX6UL_CLK_CSI>;
		clock-names = "csi_mclk";
		pwn-gpios = <&gpio_spi 6 1>;
		rst-gpios = <&gpio_spi 5 0>;
    	.......
}
    
    
pwn_gpio = of_get_named_gpio(dev->of_node, "pwn-gpios", 0);
rst_gpio = of_get_named_gpio(dev->of_node, "rst-gpios", 0);

```





## 遇到的问题

**若使用echo命令**进行测试，需要设置驱动写入函数的返回值，并将字符串转为数字

```c
ssize_t led_write (struct file * filp, const char __user * buf, size_t count, loff_t * ppos)
{
    // gpio_set_value();   //设置gpio电平
    int retvalue;
    unsigned char databuf[1];
    retvalue = copy_from_user(databuf,buf,count);
    if(retvalue<0)
    {
        printk("kernel write failed");
        return -EFAULT;
    }

    //使用echo命令测试，echo向文件写入的数据是字符串，因此需要判断输入是否有效和字符串转换
    if (databuf[0] != '0' && databuf[0] != '1') {
        printk("Invalid input: %c\n", databuf[0]);
        return -EINVAL;
    }
    // printk("databuf[0]=%u\r\n",databuf[0]);

    //databuf[0]中存储的是字符类型的'1'和'0'，对应的ASCII码是49和48,减去'0'就是实际数字了
    led_switch(databuf[0]-'0',filp);         
    
    return count;		//返回写入的字节数，若使用echo命令则必须返回函数入口参数size_t count，否则会重复执行
}
```



## ==**不能在声明cdev结构体时把它设为指针类型，不然会报错**==
