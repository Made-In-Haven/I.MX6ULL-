#include "bsp_icm20608.h"
#include "bsp_spi.h"
#include "bsp_lcd.h"
#include "bsp_lcdapi.h"
#include "stdio.h"
#include "bsp_delay.h"

struct icm20608_dev_struc icm20608_dev;

/*初始化ICM传感器*/
unsigned char icm20608_init()
{
    /*spi引脚初始化,片选信号软件控制*/
  

    IOMUXC_SetPinMux(IOMUXC_UART2_RX_DATA_ECSPI3_SCLK,0);
    IOMUXC_SetPinConfig(IOMUXC_UART2_RX_DATA_ECSPI3_SCLK,0X10B1);

    
    IOMUXC_SetPinMux(IOMUXC_UART2_RTS_B_ECSPI3_MISO,0);
    IOMUXC_SetPinConfig(IOMUXC_UART2_RTS_B_ECSPI3_MISO,0X10B1);

    IOMUXC_SetPinMux(IOMUXC_UART2_CTS_B_ECSPI3_MOSI,0);
    IOMUXC_SetPinConfig(IOMUXC_UART2_CTS_B_ECSPI3_MOSI,0X10B1);

    /*CS引脚配置*/
    IOMUXC_SetPinMux(IOMUXC_UART2_TX_DATA_GPIO1_IO20,0);
    IOMUXC_SetPinConfig(IOMUXC_UART2_TX_DATA_GPIO1_IO20,0X10B0);


    _gpio_pin_config_t gpio_config;
    gpio_config.direction = kGPIO_Digitaloutput;
    gpio_config.outputLogic = 1;    /*默认输出为1*/
    gpio_config.interruptMode = kgpio_Nointmode;
    gpio_init(GPIO1,20,&gpio_config);

    /*spi外设初始化*/
    spi_init(ECSPI3);

    /*icm20608初始化*/
    icm20608_write(ICM20_PWR_MGMT_1, 0x80);		/* 复位，复位后为0x40,睡眠模式 			*/
	delay_ms(50);
	icm20608_write(ICM20_PWR_MGMT_1, 0x01);		/* 关闭睡眠，自动选择时钟 					*/
	delay_ms(50);

    unsigned char ID = 0;
    ID = icm20608_read(ICM20_WHO_AM_I);
    if(ID!=ICM20608M_ID)
        return 1;
    
    icm20608_write(ICM20_SMPLRT_DIV, 0x00); 	/* 输出速率是内部采样率					*/
	icm20608_write(ICM20_GYRO_CONFIG, 0x18); 	/* 陀螺仪±2000dps量程 				*/
	icm20608_write(ICM20_ACCEL_CONFIG, 0x18); 	/* 加速度计±16G量程 					*/
	icm20608_write(ICM20_CONFIG, 0x04); 		/* 陀螺仪低通滤波BW=20Hz 				*/
	icm20608_write(ICM20_ACCEL_CONFIG2, 0x04); 	/* 加速度计低通滤波BW=21.2Hz 			*/
	icm20608_write(ICM20_PWR_MGMT_2, 0x00); 	/* 打开加速度计和陀螺仪所有轴 				*/
	icm20608_write(ICM20_LP_MODE_CFG, 0x00); 	/* 关闭低功耗 						*/
	icm20608_write(ICM20_FIFO_EN, 0x00);		/* 关闭FIFO						*/
    return 0;
    
}

/*ICM spi接口读数据*/
unsigned char icm20608_read(unsigned char reg_addr)
{
    unsigned char data = 0;
    reg_addr |= 0x80;   //bit7置1表示读
    ICM20608_CSN(0);
    spich0_readwrite_byte(ECSPI3,reg_addr);
    data = spich0_readwrite_byte(ECSPI3,0xff);  
    ICM20608_CSN(1);
    return data;
}


/*ICM spi接口写数据*/
void icm20608_write(unsigned char reg_addr, unsigned char data)
{
    reg_addr &= ~0x80;   //bit7置0表示写
    ICM20608_CSN(0);
    spich0_readwrite_byte(ECSPI3,reg_addr);
    spich0_readwrite_byte(ECSPI3,data);  
    ICM20608_CSN(1);
}

/*连续读取ICM20608多个寄存器, len<2**8 */
void icm20608_read_len(unsigned char reg_addr, unsigned char *buf,unsigned char len)
{
    reg_addr|= 0x80;   //bit7置1表示读
    unsigned char i = 0;
    ICM20608_CSN(0);
    spich0_readwrite_byte(ECSPI3,reg_addr);
    for(;i<len;i++)
    {
        buf[i] = spich0_readwrite_byte(ECSPI3,0xff); 
    }
    ICM20608_CSN(1);
}

/*
 * @description : 获取陀螺仪的分辨率
 * @param		: 无
 * @return		: 获取到的分辨率
 */
float icm20608_gyro_scaleget(void)
{
	unsigned char data;
	float gyroscale;
	
	data = (icm20608_read(ICM20_GYRO_CONFIG) >> 3) & 0X3;
	switch(data) {
		case 0: 
			gyroscale = 131;
			break;
		case 1:
			gyroscale = 65.5;
			break;
		case 2:
			gyroscale = 32.8;
			break;
		case 3:
			gyroscale = 16.4;
			break;
	}
	return gyroscale;
}

/*
 * @description : 获取加速度计的分辨率
 * @param		: 无
 * @return		: 获取到的分辨率
 */
unsigned short icm20608_accel_scaleget(void)
{
	unsigned char data;
	unsigned short accelscale;
	
	data = (icm20608_read(ICM20_ACCEL_CONFIG) >> 3) & 0X3;
	switch(data) {
		case 0: 
			accelscale = 16384;
			break;
		case 1:
			accelscale = 8192;
			break;
		case 2:
			accelscale = 4096;
			break;
		case 3:
			accelscale = 2048;
			break;
	}
	return accelscale;
}



/*获取ICM20608数据*/
void icm20608_get_data()
{
    unsigned char data[14];
    icm20608_read_len(ICM20_ACCEL_XOUT_H,data,14);
    float gyroscale;
    unsigned short accescale;

    gyroscale = icm20608_gyro_scaleget();
	accescale = icm20608_accel_scaleget();

    icm20608_dev.accel_x_adc = (signed short)((data[0] << 8) | data[1]); 
	icm20608_dev.accel_y_adc = (signed short)((data[2] << 8) | data[3]); 
	icm20608_dev.accel_z_adc = (signed short)((data[4] << 8) | data[5]); 
	icm20608_dev.temp_adc    = (signed short)((data[6] << 8) | data[7]); 
	icm20608_dev.gyro_x_adc  = (signed short)((data[8] << 8) | data[9]); 
	icm20608_dev.gyro_y_adc  = (signed short)((data[10] << 8) | data[11]);
	icm20608_dev.gyro_z_adc  = (signed short)((data[12] << 8) | data[13]);

	/* 计算实际值,乘一百是因为串口不支持浮点数输出*/
	icm20608_dev.gyro_x_act = ((float)(icm20608_dev.gyro_x_adc)  / gyroscale) * 100;
	icm20608_dev.gyro_y_act = ((float)(icm20608_dev.gyro_y_adc)  / gyroscale) * 100;
	icm20608_dev.gyro_z_act = ((float)(icm20608_dev.gyro_z_adc)  / gyroscale) * 100;

	icm20608_dev.accel_x_act = ((float)(icm20608_dev.accel_x_adc) / accescale) * 100;
	icm20608_dev.accel_y_act = ((float)(icm20608_dev.accel_y_adc) / accescale) * 100;
	icm20608_dev.accel_z_act = ((float)(icm20608_dev.accel_z_adc) / accescale) * 100;

	icm20608_dev.temp_act = (((float)(icm20608_dev.temp_adc) - 25 ) / 326.8 + 25) * 100;
}