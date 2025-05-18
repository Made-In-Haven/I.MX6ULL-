# 设备树led驱动实验

内容：

1. 在 imx6ull-alientek-emmc.dts 文件中创建相应的设备节点，**并写入相应的寄存器地址**。
2. 编写驱动程序(在mmu_led基础上完成)，**获取设备树中的相应寄存器地址**。
3. **使用获取到的相应寄存器地址**来初始化 LED 所使用的 GPIO。

这个实验只是为了将初步在驱动开发中使用设备树，真正的开发不会这么干



**of_iomap函数可以完成从设备数读取reg属性，并把reg属性映射为虚拟地址。示例如下：**

```c
reg = <	0X020C406C 0x04				//CCM_CCGR1_BASE
        0X020E0068 0x04				//SW_MUX_GPIO1_IO03_BASE
        0X020E02F4 0x04				//SW_PAD_GPIO1_IO03_BASE
        0X0209C000 0x04				//GPIO1_DR
        0X0209C004 0x04>;			//GPIO1_GDIR
```



```c
led.IMX6ULL_CCM_CCGR1 = of_iomap(led.node,0);
led.IMX6ULL_SW_MUX_GPIO1_IO03 = of_iomap(led.node,1);
led.IMX6ULL_SW_PAD_GPIO1_IO03 = of_iomap(led.node,2);
led.IMX6ULL_GPIO1_DR = of_iomap(led.node,3);
led.IMX6ULL_GPIO1_GDIR = of_iomap(led.node,4);
```

