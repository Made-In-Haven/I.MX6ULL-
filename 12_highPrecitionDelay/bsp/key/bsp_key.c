/*key0 连UART1_CTS复用为GPIO1_IO18
 接上拉电阻，按下为低，常态为高

 中断号为67+32=99
*/
#include "bsp_key.h"



void key_init(void)
{	
	 _gpio_pin_config_t key_config;
	
	/* 1、初始化IO复用, 复用为GPIO1_IO18 */
	IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18,0);

	/* 2、、配置UART1_CTS_B的IO属性	
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 11 默认22K上拉
	 *bit [13]: 1 pull功能
	 *bit [12]: 1 pull/keeper使能
	 *bit [11]: 0 关闭开路输出
	 *bit [7:6]: 10 速度100Mhz
	 *bit [5:3]: 000 关闭输出
	 *bit [0]: 0 低转换率
	 */
	IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18,0xF080);
	
	/* 3、初始化GPIO */
	key_config.direction = kGPIO_Digitalinput;
	gpio_init(GPIO1,18, &key_config);
	
}


int read_key()
{
    int ret = 0;
    ret = ((GPIO1->DR) >> 18)& 0x1;
    return ret;

}
