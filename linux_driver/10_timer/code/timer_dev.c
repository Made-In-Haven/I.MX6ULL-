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
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/ioctl.h>

#define DEV_NAME "led"
#define DEV_ON 0
#define DEV_OFF 1

/*ioctl指令宏定义*/
#define CMD_BASE 0XEF
#define CLOSE_CMD   (_IO(CMD_BASE, 1))    //关闭命令
#define OPEN_CMD   (_IO(CMD_BASE, 2))
#define SET_PEROID_CMD   (_IOW(CMD_BASE, 3, int))
#define PEROID (500)      //500ms





static struct dev
{
    dev_t devid;
    u32 major;
    u32 minor;
    struct cdev cdev;
    struct device* device;
    struct class* class;
    struct device_node *nd;
    struct timer_list timer;
    u32 peroid;
    u32 gpio;
    
}dev;


static int dev_open (struct inode * inode, struct file * filp)
{
    filp->private_data = &dev;
    return 0;
}
static int dev_release (struct inode * inode, struct file * filp)
{
    
    return 0;
}

/*
*   filp:文件指针
*   cmd：命令
*   arg: 参数
*
*/
long timer_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    struct dev* dev = (struct dev*)filp->private_data;
    switch (cmd)
    {
        case CLOSE_CMD:
            del_timer_sync(&dev->timer);
            break;

        case OPEN_CMD:
            ret = mod_timer(&dev->timer,jiffies + msecs_to_jiffies(PEROID));
            if(ret<0)
            {
                printk("OPEN_CMD_mod_timer_failed\r\n");
            }
            break;  

        case SET_PEROID_CMD:
            dev->peroid = arg;
            del_timer_sync(&dev->timer);
            ret = mod_timer(&dev->timer,jiffies + msecs_to_jiffies(arg));
            if(ret<0)
            {
                printk("SET_PEROID_CMD_mod_timer_failed\r\n");
            }
            break;
        default:
            del_timer_sync(&dev->timer);
            break;
    }
    return ret;
}



static const struct file_operations dev_opts = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .unlocked_ioctl = timer_ioctl
};

void timer_function(unsigned long data)
{
    struct dev* dev = (struct dev*)data;
    
    static int count = 0;
    if(count%2==0)
    {
        gpio_set_value(dev->gpio,DEV_ON);
    }
    else{
        gpio_set_value(dev->gpio,DEV_OFF);
    }
    count++;
    mod_timer(&dev->timer,jiffies + msecs_to_jiffies(dev->peroid));
}

static int led_init(struct dev* dev)
{
    int ret;

    //获取设备节点
    dev->nd =  of_find_node_by_path("/led");
    if(dev->nd==NULL)
    {
            printk("find_node_failed\r\n");
            ret = -EINVAL;
            goto find_node_failed;
    }


    //获取GPIO编号
    dev->gpio = of_get_named_gpio(dev->nd,"led-gpio",0);
    if(dev->gpio<0)
    {
            printk("get_named_gpio_failed\r\n");
            ret = -EINVAL;
            goto find_node_failed;
    }

    //申请GPIO
    ret = gpio_request(dev->gpio,"led");
    if(ret<0)    
    {
        printk("gpio_request_failed\r\n");
        ret = -EINVAL;
        goto find_node_failed;
    }

    //设置gpio默认为输出1
    ret = gpio_direction_output(dev->gpio,1);
    if(ret<0)    
    {
        printk("gpio_set_output_failed\r\n");
        ret = -EINVAL;
        goto gpio_set_output_failed;
    }

   return 0;

gpio_set_output_failed:
    gpio_free(dev->gpio);    //释放GPIO

find_node_failed:


    return ret;
}



static void timer_init(void)
{

    init_timer(&dev.timer);              //初始化定时器
    dev.timer.function = timer_function;//设置定时器回调函数
    dev.peroid = PEROID;
    dev.timer.expires = jiffies + msecs_to_jiffies(dev.peroid);    //设置超时点
    dev.timer.data = (u64)(u32)&dev;
    add_timer(&dev.timer);               //启动定时器

}




static int __init alientek_dev_init(void)
{
    int ret = 0;


    //申请设备号
    ret = alloc_chrdev_region(&(dev.devid),0,1,DEV_NAME);
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
    // printk("ret=%d\r\n",ret);
    if(ret<0)
    {
        printk("cdev_add faided\r\n");
        goto cdev_init_faided;
    }

    //初始化device和class

    dev.class = class_create(THIS_MODULE,DEV_NAME);
    if(dev.class==NULL)
    {
        printk("class创建失败\r\n");
        goto class_create_faided;
    }
    // printk("class_create\r\n");
    dev.device = device_create(dev.class, NULL, dev.devid, NULL, DEV_NAME);
    if(dev.device==NULL)
    {
        printk("device创建失败\r\n");
        goto device_create_faided;
    }
    // printk("device_create\r\n");  


    //设备初始化
    ret = led_init(&dev);
    if(ret<0)
    {
        printk("led_init_failed\r\n");
        goto dev_init_faided;
    }

    //内核定时器初始化
    timer_init();

    return 0;



dev_init_faided:
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
    //删除内核定时器
    del_timer(&dev.timer);
    // printk("del_timer\r\n");
    
    //关闭设备
    gpio_set_value(dev.gpio,DEV_OFF);
    // printk("gpio_set_value\r\n");


    //释放IO
    gpio_free(dev.gpio);
    // printk("gpio_free\r\n");

    //删除device和class类
    device_destroy(dev.class,dev.devid);
    // printk("device_destroy\r\n");
    class_destroy(dev.class);
    // printk("class_destroy\r\n");

    //卸载设备
    cdev_del(&dev.cdev);

    //释放设备号
    unregister_chrdev_region(dev.devid,1);

}


module_init(alientek_dev_init);
module_exit(alientek_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DYF");