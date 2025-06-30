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

#define LED_NAME "led"
#define LED_ON 1
#define LED_OFF 0




static struct pinctl_led_dev
{
    dev_t devid;            //设备号
    u32 major;              //主设备号
    u32 minor;              //次设备号
    struct cdev cdev;       //
    struct device* device;
    struct class* class;
    struct device_node *nd;
    u32 led_gpio;
    atomic_t lock;          //原子操作
    struct mutex mutex;

    
}led;

void led_switch(unsigned char state,struct file * filp)
{
    // printk("led_switch\r\n");
    switch (state)
    {
    case LED_ON:
        /* code */
        gpio_set_value(((struct pinctl_led_dev*)(filp->private_data))->led_gpio,0);
        break;

    case LED_OFF:
        gpio_set_value(((struct pinctl_led_dev*)(filp->private_data))->led_gpio,1);
        break;
    
    default:
        gpio_set_value(((struct pinctl_led_dev*)(filp->private_data))->led_gpio,1);
        break;
    }
}


ssize_t led_write (struct file * filp, const char __user * buf, size_t count, loff_t * ppos)
{
    // gpio_set_value();   //设置gpio电平
    int retvalue;
    unsigned char databuf[1];
    retvalue = copy_from_user(databuf,buf,count);
    if(retvalue<0)
    {
        printk("kernel write failed");
        return -EFAULT;
    }

    // printk("databuf[0]=%u\r\n",databuf[0]);

    
    led_switch(databuf[0],filp);         
    
    return count;
}
int led_open (struct inode * inode, struct file * filp)
{
    filp->private_data = &led;
    mutex_lock(&(((struct pinctl_led_dev*)(filp->private_data))->mutex));
    printk("线程进入,上锁\r\n");
    return 0;
}
int led_release (struct inode * inode, struct file * filp)
{
    filp->private_data = &led;
    mutex_unlock(&(((struct pinctl_led_dev*)(filp->private_data))->mutex));
    printk("线程退出，解锁\r\n");
    return 0;
}



static const struct file_operations led_opts = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
    .release = led_release,
};




static int __init pinctl_led_dev_init(void)
{
    int ret = 0;


    //申请设备号
    ret = alloc_chrdev_region(&(led.devid),0,1,LED_NAME);
    if(ret<0)
    {
        printk("申请设备号失败\r\n");
        goto apply_for_devid_failed;
    }

    led.major = MAJOR(led.devid);
    led.minor = MINOR(led.devid);
    // printk("led major = %u\r\n",led.major);


    //设备结构体初始化
    led.cdev.owner = THIS_MODULE;
    cdev_init(&led.cdev,&led_opts);

    //向linux注册设备
    ret = cdev_add(&led.cdev,led.devid,1);
    if(ret<0)
    {
        printk("cdev_add failed\r\n");
        goto cdev_init_failed;
    }

    //初始化device和class

    led.class = class_create(THIS_MODULE,LED_NAME);
    if(led.class==NULL)
    {
        printk("class创建失败\r\n");
        goto class_create_failed;
    }
    // printk("class_create\r\n");
    led.device = device_create(led.class, NULL, led.devid, NULL, LED_NAME);
    if(led.device==NULL)
    {
        printk("device创建失败\r\n");
        goto device_create_failed;
    }
    // printk("device_create\r\n");

    //查找设备节点
    led.nd = of_find_node_by_path("/led");      
    if(led.nd==NULL)
    {
        printk("设备节点查找失败\r\n");
        goto find_node_failed;
    }

    //获得IO编号
    led.led_gpio = of_get_named_gpio(led.nd,"led-gpio",0);  
    if(led.led_gpio<0)
    {
        printk("led-gpio查找失败\r\n");
        goto find_node_failed;
    }
    printk("led-gpio number:%u\r\n",led.led_gpio);


    //申请IO    
    ret = gpio_request(led.led_gpio,"led-gpio");    
    if(ret)
    {
        printk("failed to request GPIO for led \r\n");
        goto find_node_failed;
    }


    //IO设置
    ret = gpio_direction_output(led.led_gpio,1);    //默认输出高电平，led灭
    if(ret)
    {
        printk("failed to set GPIO for led \r\n");
        goto set_gpio_failed;
    }

    atomic_set(&(led.lock),1);
    mutex_init(&(led.mutex));
    return 0;

set_gpio_failed:
    gpio_free(led.led_gpio);    //释放IO


find_node_failed:
    device_destroy(led.class,led.devid);         //删除设备

device_create_failed:
    class_destroy(led.class);                   //删除类

class_create_failed:
    cdev_del(&led.cdev);                         //卸载设备 
    

cdev_init_failed:
    unregister_chrdev_region(led.devid,1);      //释放设备号


apply_for_devid_failed:
    return ret;
}

static void __exit pinctl_led_dev_exit(void)
{
    //关闭设备
    gpio_set_value(led.led_gpio,1);

    //释放IO
    gpio_free(led.led_gpio);

    //删除device和class类
    device_destroy(led.class,led.devid);
    class_destroy(led.class);

    //卸载设备
    cdev_del(&led.cdev);

    //释放设备号
    unregister_chrdev_region(led.devid,1);

}


module_init(pinctl_led_dev_init);
module_exit(pinctl_led_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DYF");