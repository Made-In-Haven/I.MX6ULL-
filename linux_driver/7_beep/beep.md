## 蜂鸣器实验

蜂鸣器使用的IO是**SNVS组的SNVS_TAMPER1**，与上节实验不同，**上节实验使用的是普通IO，需要在iomux**节点下创建pinctl。**蜂鸣器需要在iomuxc_snvs**下创建pinctl

```c
&iomuxc_snvs {
	.......
    pinctrl_beep:beepgrp{
			fsl,pins = <
			MX6ULL_PAD_SNVS_TAMPER1__GPIO5_IO0 		0X10B0
			>;
		};    
}
```

`MX6ULL_PAD_SNVS_TAMPER1__GPIO5_IO0`的定义在imx6ull-pinfunc-snvs.h文件中。



