/*ap3216c设备驱动代码*/
#include "bsp_ap3216c.h"
#include "bsp_i2c.h"
#include "bsp_gpio.h"
#include "stdio.h"
#include "bsp_delay.h"

/*初始化ap3216c*/
void ap3216c_init(I2C_Type * base)
{
    /*IO初始化*/
    IOMUXC_SetPinMux(IOMUXC_UART4_TX_DATA_I2C1_SCL,1);
    IOMUXC_SetPinConfig(IOMUXC_UART4_TX_DATA_I2C1_SCL,0X70B0);
    IOMUXC_SetPinMux(IOMUXC_UART4_RX_DATA_I2C1_SDA,1);
    IOMUXC_SetPinConfig(IOMUXC_UART4_RX_DATA_I2C1_SDA,0X70B0);

    /*I2C接口初始化*/
    i2c_init(base);
    /*传感器初始化*/
    unsigned char data = 0x4;
    ap3216c_write_one_byte(base,&data,AP3216C_ADDR,AP3216C_SYSTEM_CONFIG); //复位
    delay_ms(50);

    //设置ap3216c模式为ALS+PS+IR
    data = 0x3;
    ap3216c_write_one_byte(base,&data,AP3216C_ADDR,AP3216C_SYSTEM_CONFIG); 


}

/*AP3216C读一个字节数据,返回值是错误类型*/
unsigned char ap3216c_read_one_byte(I2C_Type *base, unsigned char * data, 
                                    unsigned char dev_addr, unsigned char reg_addr)
{
    struct i2c_transfer trans;
    unsigned char status;
    trans.slaveAddress = dev_addr;
    trans.direction = kI2C_Read;
    trans.subaddress = reg_addr;
    trans.subaddressSize = 1;
    trans.dataSize = 1;
    trans.data = data;
    status = i2c_master_transfer(base,&trans);
    return status;
}

/*AP3216C写一个字节数据，返回值是错误类型*/
unsigned char ap3216c_write_one_byte(I2C_Type *base, unsigned char * data, 
                                    unsigned char dev_addr, unsigned char reg_addr)
{
    struct i2c_transfer trans;
    unsigned char status;
    trans.slaveAddress = dev_addr;
    trans.direction = kI2C_Write;
    trans.subaddress = reg_addr;
    trans.subaddressSize = 1;
    trans.dataSize = 1;
    trans.data = data;
    status = i2c_master_transfer(base,&trans);
    return status;
}



/*ap3216c数据读取*/
void ap3216c_read_data(I2C_Type * base,unsigned short * ir, unsigned short * als, unsigned short *ps)
{
    unsigned char buf[6];
    unsigned char i = 0;
    for(i=0;i<6;i++)
    {
        ap3216c_read_one_byte(base,(buf+i),AP3216C_ADDR,
                                        AP3216C_IR_DATA_LOW+i);
    }
    if(buf[0]&(1<<7))   //IR PS数据无效
    {
        *ir = 0;
        *ps = 0;
    }
    else
    {
        *ir = (buf[0]&0x3)|((unsigned short)buf[1]<<2);
        *ps = (buf[4]&0xf)|(((unsigned short)buf[5] & 0x3f)<<4);
    }
    *als = buf[2]|((unsigned short)buf[3]<<8);

}

/*ap3216c读取als数据*/
void ap3216c_read_als(I2C_Type * base,unsigned short * als)
{
    unsigned char buf[2] = {0,0};
    ap3216c_read_one_byte(base,&(buf[0]),AP3216C_ADDR,
                        AP3216C_ALS_DATA_LOW);
    printf("%d\r\n",buf[0]);
    ap3216c_read_one_byte(base,&(buf[1]),AP3216C_ADDR,
                        AP3216C_ALS_DATA_HIGH);
    printf("%d\r\n",buf[1]);

    *als = buf[0]|((unsigned short)buf[1]<<8);

}
