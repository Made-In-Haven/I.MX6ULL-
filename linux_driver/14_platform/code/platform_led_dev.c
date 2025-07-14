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

#define DEV_NAME "led"
#define DEV_ON 1
#define DEV_OFF 0


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
    spinlock_t spinlock;

}dev;

static struct of_device_id led_of_match[] = {
    {.compatible = "alientek_imx6ull_led"},
    {}
};

void led_switch(char state)
{
    switch(state)
    {
        case DEV_ON:
            gpio_set_value(dev.gpio,0);
            break;

        case DEV_OFF:
            gpio_set_value(dev.gpio,1);
            break;

        default:
            break;
    }
}



static ssize_t dev_write (struct file * filp, const char __user * buf, size_t count, loff_t * ppos)
{
    int ret = 0;
    char data_buf[1];
    struct dev* dev = (struct dev*)filp->private_data;
    ret = copy_from_user(data_buf,buf,sizeof(data_buf));
    spin_lock(&dev->spinlock);
    led_switch(data_buf[0]-'0');
    spin_unlock(&dev->spinlock);
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



int led_init(struct device* device_led)
{
    int ret=0;
    dev.nd = device_led->of_node;
    // dev.nd = of_find_node_by_path("/led");
    if(dev.nd==NULL)
    {
        printk("find_node_failed\r\n");
        ret = -ENAVAIL;
        goto find_node_failed;
    }
    dev.gpio = of_get_named_gpio(dev.nd,"led-gpio",0);
    if(dev.gpio<0)
    {
        printk("get_gpio_failed\r\n");
        ret = -ENAVAIL;
        goto get_gpio_failed;
    }
    ret = gpio_request(dev.gpio,"led-gpio");
    if(ret<0)
    {
        printk("gpio_request_failed\r\n");
        goto gpio_request_failed;
    }
    ret = gpio_direction_output(dev.gpio,1);
    if(ret<0)
    {
        printk("set_direction_failed\r\n");
        goto set_direction_failed;
    }

    return 0;

set_direction_failed:
    gpio_free(dev.gpio);

gpio_request_failed:


get_gpio_failed:


find_node_failed:

    return ret;

}

static int led_probe(struct platform_device *device)
{
    
    int ret = 0;
    struct device device_led = device->dev;
    printk("led_probe\r\n");

    ret = alloc_chrdev_region(&dev.devid,0,1,DEV_NAME);
    if(ret<0)
    {
        printk("alloc_chrdev_region_faid\r\n");
        goto alloc_chrdev_region_faid;
    }
    dev.major = MAJOR(dev.devid);
    dev.minor = MINOR(dev.devid);

    dev.cdev.owner = THIS_MODULE;
    cdev_init(&dev.cdev,&dev_opts);

    ret = cdev_add(&dev.cdev,dev.devid,1);
    if(ret<0)
    {
        printk("cdev_add_failed\r\n");
        goto cdev_add_failed;
    }

    //初始化class和device
    dev.class = class_create(THIS_MODULE,DEV_NAME);
    if(dev.class==NULL)
    {
        printk("class_create_failed\r\n");
        ret = -ENAVAIL;
        goto class_create_failed;
    }
    dev.device = device_create(dev.class,NULL,dev.devid,NULL,DEV_NAME);
    if(dev.device==NULL)
    {
        printk("device_create_failed\r\n");
        ret = -ENAVAIL;
        goto device_create_failed;
    }

    ret = led_init(&device_led);
    if(ret<0)
    {
        printk("led_init_failed\r\n");
        goto led_init_failed;
    }
    spin_lock_init(&dev.spinlock);


    return 0;

led_init_failed:


device_create_failed:
    class_destroy(dev.class);

class_create_failed:
    cdev_del(&dev.cdev);


cdev_add_failed:
    unregister_chrdev_region(dev.devid,1);

alloc_chrdev_region_faid:

    return ret;

}

static int led_remove(struct platform_device *device)
{
    led_switch(DEV_OFF);

    gpio_free(dev.gpio);

    device_destroy(dev.class,dev.devid);
    
    class_destroy(dev.class);

    cdev_del(&dev.cdev);

    unregister_chrdev_region(dev.devid,1);

    return 0;
}





struct platform_driver led_driver = {
    .remove = led_remove,
    .probe = led_probe,
    .driver.of_match_table = led_of_match,
    .driver.name = "imx6ull_led"
};




static int __init alientek_dev_init(void)
{
    
    return platform_driver_register(&led_driver);
}

static void __exit alientek_dev_exit(void)
{
    platform_driver_unregister(&led_driver);
}


module_init(alientek_dev_init);
module_exit(alientek_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DYF");