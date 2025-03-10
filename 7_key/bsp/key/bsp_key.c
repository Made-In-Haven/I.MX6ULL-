/*key0 连UART1_CTS
* 接上拉电阻，按下为低，常态为高
*/
#include "bsp_key.h"

void key_init()
{
    _gpio_pin_config_t pin_config;
    pin_config.direction = kGPIO_Digitalinput;
    gpio_init(GPIO1, 18, &pin_config);
}



int read_key()
{
    int ret = 0;
    ret = ((GPIO1->DR) >> 18)& 0x1;
    return ret;

}

int key_getValue()
{
    int ret = 0;
    static unsigned char release = 1; /*为1表示按键被释放*/

    if(release==1&&(read_key() == 0))
    {
        delay(10);
        release = 0;
        if(read_key()==0)
        {
            ret = KEY0_VALUE;
        }
        

    }
    else if(read_key() == 1)
        {
            release = 1;
            ret = 0;
        }
    return ret;
}