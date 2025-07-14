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
#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#define DEV_NAME "beep"
#define BEEP_ON 1
#define BEEP_OFF 0



struct dev{
    dev_t dev_id;
    char major;
    char minor;
    struct device_node *nd;
    spinlock_t spinlock;
    u32 gpio;
};
static struct dev dev;

void beep_switch(unsigned char state,struct file * filp)
{
    // printk("beep_switch:%d:\r\n",state);
    switch (state)
    {
    case BEEP_ON:
        /* code */
        gpio_set_value(((struct dev*)(filp->private_data))->gpio,0);
        break;

    case BEEP_OFF:
        gpio_set_value(((struct dev*)(filp->private_data))->gpio,1);
        break;
    
    default:
        gpio_set_value(((struct dev*)(filp->private_data))->gpio,1);
        break;
    }
}

ssize_t misc_dev_write (struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
    int ret = 0;
    char data_buf[1];
    struct dev* dev = (struct dev*)filp->private_data;
    // printk("misc_dev_write\r\n");

    ret = copy_from_user(data_buf,buf,sizeof(data_buf));
    spin_lock(&dev->spinlock);
    beep_switch(data_buf[0]-'0',filp);
    spin_unlock(&dev->spinlock);
    return count;
}
int misc_dev_open (struct inode *inode, struct file *filp)
{
    filp->private_data = &dev;
    return 0;
}
int misc_dev_close (struct inode *inode, struct file *filp)
{
    return 0;
}

static struct file_operations dev_opts = {
    .write = misc_dev_write,
    .release = misc_dev_close,
    .open = misc_dev_open
};

static struct miscdevice misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME,
    .fops = &dev_opts
};

int misc_dev_probe(struct platform_device *platform_device)
{
    int ret = 0;
    struct device device = platform_device->dev;
    ret = misc_register(&misc_dev);
    if(ret<0)
    {
        printk("misc_register_failed\r\n");
        goto misc_register_failed;

    }

    //初始化设备
    spin_lock_init(&dev.spinlock);
    dev.nd = device.of_node;
    if(dev.nd==NULL)
    {
        ret = -ENAVAIL;
        printk("of_get_node_failed\r\n");
        goto of_get_node_failed;
    }
    dev.gpio = of_get_named_gpio(dev.nd,"beep-gpio",0);
    if(dev.gpio<0)
    {
        ret = -ENAVAIL;
        printk("of_get_gpio_failed\r\n");
        goto of_get_node_failed;
    }
    ret = gpio_request(dev.gpio,"beep-gpio");
    if(ret<0)
    {
        printk("gpio_request_failed\r\n");
        goto of_get_node_failed;
    }
    ret = gpio_direction_output(dev.gpio,1);  //默认输出高电平，beep灭
    if(ret<0)
    {
        printk("set_direction_failed\r\n");
        goto set_direction_failed;
    }
    return 0;

set_direction_failed:
    gpio_free(dev.gpio);

of_get_node_failed:
    misc_deregister(&misc_dev);

misc_register_failed:

    return ret;
}

int misc_dev_remove(struct platform_device *platform_device)
{
    int ret = 0;

    gpio_free(dev.gpio);

    ret = misc_deregister(&misc_dev);
    if(ret<0)
    {
        printk("misc_deregister_failed\r\n");
        return ret;
    }
    return 0;
}



static struct of_device_id dev_match_table[] = {
    {.compatible = "beep-alientek"},
    {}
}; 

static struct platform_driver misc_dev_driver = {
    .probe = misc_dev_probe,
    .remove = misc_dev_remove,
    .driver = {
        .of_match_table = dev_match_table,
        .name = "beep"
    }
};



static int __init misc_dev_init(void)
{
    return platform_driver_register(&misc_dev_driver);
}

static void __exit misc_dev_exit(void)
{
    platform_driver_unregister(&misc_dev_driver);
}

module_init(misc_dev_init);
module_exit(misc_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DYF");