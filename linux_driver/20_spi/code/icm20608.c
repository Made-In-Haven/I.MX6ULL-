#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/spi/spi.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>        
#include <linux/miscdevice.h>
#include "icm20608.h"


#define DEV_NAME "icm20608"
#define WRITE_MAX_SIZE 256


struct icm20608_dev{
    dev_t dev_id;               //设备号
    struct device_node *node;   //设备节点
    struct device *device;      
    int minor;                  //从设备号
    void* private_data;         //私有数据,对于这个驱动是icm20608的结构体
    signed int accel_x_adc,accel_y_adc,accel_z_adc,temp_adc, \
                gyro_x_adc,gyro_y_adc,gyro_z_adc;     //传感器数据

    spinlock_t spinlock;        //读取自旋锁
    
    int cs_gpio;                //软件片选引脚

};

static struct icm20608_dev icm20608;


/*
 * @description :   从icm20608中读取多个寄存器数据,若len>1,则从reg寄存器地址开始读len个寄存器
 * @param dev :   自定义的icm20608设备
 * @param reg :   要读取的寄存器地址
 * @param val :   存放读取到的数据的缓冲区
 * @param len :   要读取的数据长度
 * @return      :   操作结果，正常为0，不正常为负数
 */
static int icm20608_read_regs(struct icm20608_dev *dev, u8 reg, void *val, int len)
{

    int ret = 0;
    int i = 0;
    unsigned char *txbuf = NULL;  // 发送缓冲区（命令 + dummy）
    unsigned char *rxbuf = NULL;  // 接收缓冲区（垃圾 + 有效数据）
    struct spi_transfer *transfer = NULL;
    struct spi_message m;

    struct spi_device *spi_dev = (struct spi_device *)dev->private_data;

    // 1. 分配 spi_transfer
    transfer = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
    if (!transfer) {
        printk("kmalloc transfer failed\n");
        return -ENOMEM;
    }

    // 2. 分配发送缓冲区：len + 1 字节
    txbuf = kzalloc((len + 1) * sizeof(unsigned char), GFP_KERNEL);
    if (!txbuf) {
        ret = -ENOMEM;
        printk("kmalloc txbuf failed\n");
        goto txbuf_failed;
    }

    // 3. 分配接收缓冲区：len + 1 字节
    rxbuf = kzalloc((len + 1) * sizeof(unsigned char), GFP_KERNEL);
    if (!rxbuf) {
        ret = -ENOMEM;
        printk("kmalloc rxbuf failed\n");
        goto rxbuf_failed;
    }

    // 4. 设置发送缓冲区：命令字节 + dummy
    txbuf[0] = reg | 0x80;  // 读命令，最高位置1
    // 后续字节设为0（dummy），无需额外设置（kzalloc已初始化为0）

    // 5. 配置SPI传输
    transfer->tx_buf = txbuf;
    transfer->rx_buf = rxbuf;
    transfer->len = len + 1;  // 总长度：1字节命令 + len字节dummy/接收
    spi_message_init(&m);
    spi_message_add_tail(transfer, &m);

    // 6. 执行SPI传输
    ret = spi_sync(spi_dev, &m);
    if (ret) {
        printk("spi读取失败\n");
        goto spi_sync_failed;
    }

    // 7. 复制有效数据：跳过接收缓冲区的第一个垃圾字节
    memcpy(val, &rxbuf[1], len * sizeof(unsigned char));

    // 可选：调试打印（实际数据从 rxbuf[1] 开始）
    // printk("received data: ");
    // for (; i < len+1; i++) {
    //     printk("%#x ", rxbuf[i]);
    // }
    // printk("\n");

spi_sync_failed:
    kfree(rxbuf);  // 释放接收缓冲区

rxbuf_failed:
    kfree(txbuf);  // 释放发送缓冲区

txbuf_failed:
    kfree(transfer);  // 释放传输结构体

    return ret;

}

/*
 * @description :   向icm20608写数据
 * @param dev :   自定义的icm20608设备
 * @param reg :   要写入的寄存器地址
 * @param buf :   要写入的数据
 * @param len :   要写入的数据长度
 * @return    :   操作结果，正常为0，不正常为负数
 */
static int icm20608_write_regs(struct icm20608_dev *dev, u8 reg, u8 *val, int len)
{
    int ret = 0;
    unsigned char *txbuf = NULL;  // 发送缓冲区（命令 + dummy）
    unsigned char *rxbuf = NULL;  // 接收缓冲区
    struct spi_transfer *transfer = NULL;
    struct spi_message m;

    struct spi_device *spi_dev = (struct spi_device *)dev->private_data;

    // 1. 分配 spi_transfer
    transfer = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
    if (!transfer) {
        printk("kmalloc transfer failed\n");
        return -ENOMEM;
    }

    // 2. 分配发送缓冲区：len + 1 字节
    txbuf = kzalloc((len + 1) * sizeof(unsigned char), GFP_KERNEL);
    if (!txbuf) {
        ret = -ENOMEM;
        printk("kmalloc txbuf failed\n");
        goto txbuf_failed;
    }

    // 3. 分配接收缓冲区：len + 1 字节
    rxbuf = kzalloc((len + 1) * sizeof(unsigned char), GFP_KERNEL);
    if (!rxbuf) {
        ret = -ENOMEM;
        printk("kmalloc rxbuf failed\n");
        goto rxbuf_failed;
    }

    // 4. 设置发送缓冲区：命令字节 + dummy
    txbuf[0] = reg & ~0x80;  // 写命令，最高位置0
    // 后续字节设为0（dummy），无需额外设置（kzalloc已初始化为0）

    memcpy(txbuf+1,val,len);

    // 5. 配置SPI传输
    transfer->tx_buf = txbuf;
    transfer->rx_buf = rxbuf;
    transfer->len = len + 1;  // 总长度：1字节命令 + len字节发送
    spi_message_init(&m);
    spi_message_add_tail(transfer, &m);

    // 6. 执行SPI传输
    ret = spi_sync(spi_dev, &m);
    if (ret) {
        printk("spi读取失败\n");
        goto spi_sync_failed;
    }

spi_sync_failed:
    kfree(rxbuf);  // 释放接收缓冲区

rxbuf_failed:
    kfree(txbuf);  // 释放发送缓冲区

txbuf_failed:
    kfree(transfer);  // 释放传输结构体

    return ret;
}

static int icm20608_write_onereg(struct icm20608_dev *dev, u8 reg, u8 val)
{
    int ret = 0;
    ret = icm20608_write_regs(dev,reg,&val,1);
    return ret;
}


/*
 * @description :   从icm20608中读取数据
 * @param dev   :   自定义的设备结构体
 * @return      :   负数：读取异常
 */
static int icm20608_read_data(struct icm20608_dev *dev)
{
    int ret = 0;
    unsigned char data[14];
    memset(data,0,sizeof(data));
    ret = icm20608_read_regs(dev, ICM20_ACCEL_XOUT_H,data,14);
    dev->accel_x_adc = (signed short)((data[0] << 8) | data[1]); 
	dev->accel_y_adc = (signed short)((data[2] << 8) | data[3]); 
	dev->accel_z_adc = (signed short)((data[4] << 8) | data[5]); 
	dev->temp_adc    = (signed short)((data[6] << 8) | data[7]); 
	dev->gyro_x_adc  = (signed short)((data[8] << 8) | data[9]); 
	dev->gyro_y_adc  = (signed short)((data[10] << 8) | data[11]);
	dev->gyro_z_adc  = (signed short)((data[12] << 8) | data[13]);
    // printk("accel_x_adc:%ud\r\n",dev->accel_x_adc);
    
    return ret;
}

//字符设备操作函数集

static ssize_t icm20608_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    signed int data[7];
    struct icm20608_dev* dev = (struct icm20608_dev*)filp->private_data;
    icm20608_read_data(dev);
    data[0] =   dev->accel_x_adc;
    data[1] =   dev->accel_y_adc;
    data[2] =   dev->accel_z_adc;
    data[3] =   dev->temp_adc  ; 
    data[4] =   dev->gyro_x_adc ;
    data[5] =   dev->gyro_y_adc ;
    data[6] =   dev->gyro_z_adc ;

    ret =  copy_to_user(buf,data,count);
    
    return count;
}
static int icm20608_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &icm20608;

    return 0;
}
static int icm20608_release(struct inode *inode, struct file *filp)
{

    return 0;
}

static struct file_operations icm20608_fops = {
    .owner = THIS_MODULE,
    .open = icm20608_open,
    .read = icm20608_read,
    .release = icm20608_release
};

static struct miscdevice icm20608_misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,  /* 动态分配次设备号 */
    .fops = &icm20608_fops,
    .name = DEV_NAME

};



//I2C设备驱动
static struct of_device_id icm20608_match_table[] = {
    {.compatible = "alientek,icm20608"},
    {}
};
static const struct spi_device_id icm20608_id[] = {
    { "icm20608", 0 },
    {}
};

/* @description : 初始化陀螺仪，陀螺仪分辨率为16.4, 加速度计分辨率位2048
 * @param       : 无
 * @return      : 0正常，负数异常
 */
int icm20608_reginit(struct spi_device *spi)
{
    int ret = 0;
    u8 data = 0;



    //设备初始化
    ret = icm20608_write_onereg(&icm20608,ICM20_PWR_MGMT_1,0x80);    /* 复位，复位后为0x40,睡眠模式 			*/
    mdelay(50);

    ret = icm20608_write_onereg(&icm20608,ICM20_PWR_MGMT_1,0x01);    /* 关闭睡眠，自动选择时钟 					*/
    mdelay(50);

    ret = icm20608_read_regs(&icm20608,ICM20_WHO_AM_I,&data,1);
    printk("icm20608 ID: %#x\r\n",data);

    
    ret = icm20608_write_onereg(&icm20608,ICM20_SMPLRT_DIV, 0x00); 	/* 输出速率是内部采样率*/
	ret = icm20608_write_onereg(&icm20608,ICM20_GYRO_CONFIG, 0x18); 	/* 陀螺仪±2000dps量程*/
	ret = icm20608_write_onereg(&icm20608,ICM20_ACCEL_CONFIG, 0x18); 	/* 加速度计±16G量程*/
	ret = icm20608_write_onereg(&icm20608,ICM20_CONFIG, 0x04); 		/* 陀螺仪低通滤波BW=20Hz */
	ret = icm20608_write_onereg(&icm20608,ICM20_ACCEL_CONFIG2, 0x04); 	/* 加速度计低通滤波BW=21.2Hz */
	ret = icm20608_write_onereg(&icm20608,ICM20_PWR_MGMT_2, 0x00); 	/* 打开加速度计和陀螺仪所有轴 				*/
	ret = icm20608_write_onereg(&icm20608,ICM20_LP_MODE_CFG, 0x00); 	/* 关闭低功耗 						*/
	ret = icm20608_write_onereg(&icm20608,ICM20_FIFO_EN, 0x00);		/* 关闭FIFO						*/

    return ret;
}


static int icm20608_probe(struct spi_device *spi)
{
    int ret = 0;

    printk("probe modalias: %s\r\n",spi->modalias);

    
    icm20608.private_data = (void*)spi;
    icm20608.device = &spi->dev;

    //misc设备注册
    ret = misc_register(&icm20608_misc_dev);
    if(ret<0)
    {
        printk("misc_register_failed\r\n");
        goto misc_register_failed;
    }


    icm20608.cs_gpio = spi->cs_gpio;

    printk("icm20608.cs_gpio : %d\r\n",icm20608.cs_gpio);




    //初始化spi
    spi->mode = SPI_MODE_0;     //设置spi为模式0，也就是CPOL=0，CPHA=0
    spi_setup(spi);

    printk("icm20608.cs_gpio value : %d\r\n",gpio_get_value(icm20608.cs_gpio));


    //设备初始化
    ret = icm20608_reginit(spi);
    if(ret)
    {
        printk("icm20608_init_failed\r\n");
        goto icm20608_init_failed;
    }



    return 0;

icm20608_init_failed:
    


gpio_direction_output_failed:
    gpio_free(icm20608.cs_gpio);

gpio_request_failed:
    misc_deregister(&icm20608_misc_dev);

misc_register_failed:

    return ret;
}

static int icm20608_remove(struct spi_device *spi)
{
    //复位设备
    icm20608_write_onereg(&icm20608,ICM20_PWR_MGMT_1,0x40);
    // gpio_free(icm20608.cs_gpio);
    misc_deregister(&icm20608_misc_dev);
    // printk("misc_deregister\r\n");
    return 0;
}

static struct spi_driver icm20608_driver = {
    .probe = icm20608_probe,
    .remove = icm20608_remove,
    .driver = {
        .name = "icm20608",
        .owner = THIS_MODULE,
        .of_match_table = icm20608_match_table,
    },
    .id_table = icm20608_id,
};







static int __init icm20608_init(void)
{
    int ret = 0;


    ret = spi_register_driver(&icm20608_driver);
    // printk("ret:%d\r\n",ret);

    return ret;
}

static void __exit icm20608_exit(void)
{
    
    spi_unregister_driver(&icm20608_driver);

}

module_init(icm20608_init);
module_exit(icm20608_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dyf");