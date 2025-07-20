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
#include <linux/i2c.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>        
#include <linux/miscdevice.h>
#include "ap3216c.h"


#define DEV_NAME "ap3216c_dev"
#define WRITE_MAX_SIZE 256


struct ap3216c_dev{
    dev_t dev_id;               //设备号
    struct device_node *node;   //设备节点
    struct device *device;      
    int minor;                  //从设备号
    void* private_data;         //私有数据,对于这个驱动是ap3216c的i2c_client结构体
    unsigned int ir,als,ps;     //三个传感器数据
    spinlock_t spinlock;        //读取自旋锁

};

static struct ap3216c_dev ap3216c;


/*
 * @description :   从ap3216c中读取多个寄存器数据,若len>1,则从reg寄存器地址开始读len个寄存器
 * @param dev :   自定义的ap3216c设备
 * @param reg :   要读取的寄存器地址
 * @param val :   存放读取到的数据的缓冲区
 * @param len :   要读取的数据长度
 * @return      :   操作结果，正常为0，不正常为负数
 */
static int ap3216c_read_regs(struct ap3216c_dev *dev, u8 reg, void *val, int len)
{
    
    int ret = 0;
    struct i2c_client *client = (struct i2c_client *)dev->private_data;
    struct i2c_msg msg[2];
    //第一次告诉ap3216c要读哪个寄存器
    msg[0].addr = client->addr;
    msg[0].flags = 0;
    msg[0].buf = &reg;
    msg[0].len = 1;

    //第二次从ap3216c读数据
    msg[1].addr = client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].buf = val;
    msg[1].len = len;

    ret = i2c_transfer(client->adapter,msg,2);

    if(ret==2){      //传输正常
        ret = 0;
    }else{
        ret = -EREMOTEIO;       //I/O操作失败
        printk("ap3216c读取数据异常\r\n");
    }
    return ret;
}

/*
 * @description :   向ap3216c写数据
 * @param dev :   自定义的ap3216c设备
 * @param reg :   要写入的寄存器地址
 * @param buf :   要写入的数据
 * @param len :   要写入的数据长度
 * @return    :   操作结果，正常为0，不正常为负数
 */
static int ap3216c_write_regs(struct ap3216c_dev *dev, u8 reg, u8 *val, int len)
{
    int ret = 0;
    u8 buf[WRITE_MAX_SIZE];     //写入数据的缓冲区
    struct i2c_msg msg;
    struct i2c_client *client = dev->private_data;

    buf[0] = reg;               //第一个写入的数据是要写的寄存器地址
    memcpy(&buf[1],val,len);    //把要写的数据拷贝到缓冲区中

    msg.addr = client->addr;
    msg.flags = 0;
    msg.buf = buf;
    msg.len = len+1;    //数据的长度：要写入数据的长度 + 1（要写入的寄存器）

    ret = i2c_transfer(client->adapter,&msg,1); //发送一次消息
    if(ret==1){
        ret = 0;
    }else{
        ret = -EREMOTEIO;
        printk("ap3216c写入数据异常");
    }

    return ret;

}

/*
 * @description :   从ap3216c中读取数据，包括ALS(数字环境光传感器),PS(接近传感器)和IR(红外LED)。读取至dev的成员变量中。
 * @param dev   :   自定义的设备结构体
 * @return      :   负数：读取异常
 */
static int ap3216c_read_data(struct ap3216c_dev *dev)
{
    int ret= 0;
    int i;

    u8 buf[6];

    for(i=0;i<6;i++)
    {
        ap3216c_read_regs(dev,AP3216C_IRDATALOW+i,buf+i,1);
    }

    if(buf[0]&(1<<7))   //IR PS数据无效
    {
        dev->ir = 0;
        dev->ps = 0;
    }
    else
    {
        dev->ir = (buf[0]&0x3)|((unsigned short)buf[1]<<2);
        dev->ps = (buf[4]&0xf)|(((unsigned short)buf[5] & 0x3f)<<4);
    }
    dev->als = buf[2]|((unsigned short)buf[3]<<8);

    return 0;
}

//字符设备操作函数集

static ssize_t ap3216c_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    struct ap3216c_dev *dev = (struct ap3216c_dev *)filp->private_data;
    short data[3];

    ap3216c_read_data(dev);

    data[0] = dev->ir;
    data[1] = dev->als;
    data[2] = dev->ps;
    ret = copy_to_user(buf,data,sizeof(data));

    return count;
}
static int ap3216c_open(struct inode *inode, struct file *filp)
{
    filp->private_data = &ap3216c;

    return 0;
}
static int ap3216c_release(struct inode *inode, struct file *filp)
{

    return 0;
}

static struct file_operations ap3216c_fops = {
    .owner = THIS_MODULE,
    .open = ap3216c_open,
    .read = ap3216c_read,
    .release = ap3216c_release
};

static struct miscdevice ap3216c_misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,  /* 动态分配次设备号 */
    .fops = &ap3216c_fops,
    .name = DEV_NAME

};



//I2C设备驱动
static struct of_device_id ap3216c_match_table[] = {
    {.compatible = "alientek,ap3216c"},
    {}
};
static const struct i2c_device_id ap3216c_id[] = {
    { "ap3216c", 0 },
    {}
};


static int ap3216c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0;
    unsigned char data;

    // printk(KERN_INFO "ap3216c_probe: client=0x%p, addr=0x%02x\n", 
    //     client, client->addr);

    ret = i2c_check_functionality(client->adapter, I2C_FUNC_I2C);
    if (!ret)
        return -ENODEV;
    
    // printk("ap3216c_probe\r\n");

    
    ap3216c.private_data = (void*)client;
    ap3216c.device = &client->dev;

    //misc设备注册
    ret = misc_register(&ap3216c_misc_dev);
    if(ret<0)
    {
        printk("misc_register_failed\r\n");
        goto misc_register_failed;
    }

    spin_lock_init(&ap3216c.spinlock);

    //设备初始化
    data = 0x4;
    ap3216c_write_regs(&ap3216c,AP3216C_SYSTEMCONG,&data,1); //复位
    mdelay(50);     //ap3216c复位至少10ms
    data = 0x3;
    ap3216c_write_regs(&ap3216c,AP3216C_SYSTEMCONG,&data,1); //开启3个传感器



    return ret;

misc_register_failed:
    return ret;
}

static int ap3216c_remove(struct i2c_client *client)
{
    u8 data = 0x4;
    // printk("ap3216c_remove\r\n");
   
    //复位设备
    ap3216c_write_regs(&ap3216c,AP3216C_SYSTEMCONG,&data,1); 

    misc_deregister(&ap3216c_misc_dev);
    // printk("misc_deregister\r\n");
    return 0;
}

static struct i2c_driver ap3216c_driver = {
    .probe = ap3216c_probe,
    .remove = ap3216c_remove,
    .driver = {
        .name = "ap3216c",
        .owner = THIS_MODULE,
        .of_match_table = ap3216c_match_table,
    },
    .id_table = ap3216c_id,
};







static int __init ap3216c_init(void)
{
    int ret = 0;

    printk("ap3216c_init\r\n");
    printk(KERN_ALERT "Hello,world\n");

    ret = i2c_add_driver(&ap3216c_driver);
    // printk("ret:%d\r\n",ret);

    return ret;
}

static void __exit ap3216c_exit(void)
{
    
    i2c_del_driver(&ap3216c_driver);

}

module_init(ap3216c_init);
module_exit(ap3216c_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dyf");