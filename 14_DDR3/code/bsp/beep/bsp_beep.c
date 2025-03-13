#include "bsp_beep.h"


/*初始化*/
void beep_init()
{
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER1,0);
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER1,0X10B0);
    GPIO5->GDIR |= (1<<1);/*设置为输出*/
    GPIO5->DR |= (1<<1);/*默认关闭*/

}
void beep_switch(int status)
{
    switch(status)
    {
        case beep_on:   GPIO5->DR &= ~(1<<1);
                        break;
        case beep_off:  GPIO5->DR |= (1<<1);
                        break;
        default:        GPIO5->DR |= (1<<1);
                        break;

    }
}