#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>

#define KEY_NAME "key"
#define KEY_ON 1
#define KEY_OFF 0


static struct dev
{
    dev_t devid;
    u32 major;
    u32 minor;
    struct cdev cdev;
    struct device* device;
    struct class* class;
    struct device_node *nd;
    u32 gpio;

    
}dev;


static ssize_t dev_write (struct file * filp, const char __user * buf, size_t count, loff_t * ppos)
{

    
    return count;
}
static int dev_open (struct inode * inode, struct file * filp)
{
    filp->private_data = &dev;
    return 0;
}
static int dev_release (struct inode * inode, struct file * filp)
{
    
    return 0;
}



static const struct file_operations dev_opts = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .write = dev_write,
    .release = dev_release,
};



static int __init alientek_dev_init(void)
{
    int ret = 0;


    //申请设备号
    ret = alloc_chrdev_region(&(dev.devid),0,1,KEY_NAME);
    if(ret<0)
    {
        printk("申请设备号失败\r\n");
        goto apply_for_devid_faided;
    }

    dev.major = MAJOR(dev.devid);
    dev.minor = MINOR(dev.devid);
    // printk("dev major = %u\r\n",dev.major);


    //设备结构体初始化
    dev.cdev.owner = THIS_MODULE;
    cdev_init(&dev.cdev,&dev_opts);

    //向linux注册设备
    ret = cdev_add(&dev.cdev,dev.devid,1);
    if(ret<0)
    {
        printk("cdev_add faided\r\n");
        goto cdev_init_faided;
    }

    //初始化device和class

    dev.class = class_create(THIS_MODULE,KEY_NAME);
    if(dev.class==NULL)
    {
        printk("class创建失败\r\n");
        goto class_create_faided;
    }
    // printk("class_create\r\n");
    dev.device = device_create(dev.class, NULL, dev.devid, NULL, KEY_NAME);
    if(dev.device==NULL)
    {
        printk("device创建失败\r\n");
        goto device_create_faided;
    }
    // printk("device_create\r\n");

    //设备初始化

    


    return 0;

set_gpio_faided:
    gpio_free(dev.gpio);    //释放IO

find_node_faided:
    device_destroy(dev.class,dev.devid);         //删除设备

device_create_faided:
    class_destroy(dev.class);                   //删除类

class_create_faided:
    cdev_del(&dev.cdev);                         //卸载设备 
    

cdev_init_faided:
    unregister_chrdev_region(dev.devid,1);      //释放设备号


apply_for_devid_faided:
    return ret;
}

static void __exit alientek_dev_exit(void)
{
    //关闭设备
    

    //释放IO
    gpio_free(dev.gpio);

    //删除device和class类
    device_destroy(dev.class,dev.devid);
    class_destroy(dev.class);

    //卸载设备
    cdev_del(&dev.cdev);

    //释放设备号
    unregister_chrdev_region(dev.devid,1);

}


module_init(alientek_dev_init);
module_exit(alientek_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DYF");