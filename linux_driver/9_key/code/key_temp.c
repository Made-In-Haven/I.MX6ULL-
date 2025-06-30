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




static struct key_dev
{
    dev_t devid;
    u32 major;
    u32 minor;
    struct cdev cdev;
    struct device* device;
    struct class* class;
    struct device_node *nd;
    u32 gpio_num;

    
}key;


static ssize_t key_write (struct file * filp, const char __user * buf, size_t count, loff_t * ppos)
{

    
    return count;
}
static int key_open (struct inode * inode, struct file * filp)
{
    filp->private_data = &key;
    return 0;
}
static int key_release (struct inode * inode, struct file * filp)
{
    
    return 0;
}



static const struct file_operations key_opts = {
    .owner = THIS_MODULE,
    .open = key_open,
    .write = key_write,
    .release = key_release,
};




static int __init alientek_key_init(void)
{
    int ret = 0;


    //申请设备号
    ret = alloc_chrdev_region(&(key.devid),0,1,KEY_NAME);
    if(ret<0)
    {
        printk("申请设备号失败\r\n");
        goto apply_for_devid_faided;
    }

    key.major = MAJOR(key.devid);
    key.minor = MINOR(key.devid);
    // printk("key major = %u\r\n",key.major);


    //设备结构体初始化
    key.cdev.owner = THIS_MODULE;
    cdev_init(&key.cdev,&key_opts);

    //向linux注册设备
    ret = cdev_add(&key.cdev,key.devid,1);
    if(ret<0)
    {
        printk("cdev_add faided\r\n");
        goto cdev_init_faided;
    }

    //初始化device和class

    key.class = class_create(THIS_MODULE,KEY_NAME);
    if(key.class==NULL)
    {
        printk("class创建失败\r\n");
        goto class_create_faided;
    }
    // printk("class_create\r\n");
    key.device = device_create(key.class, NULL, key.devid, NULL, KEY_NAME);
    if(key.device==NULL)
    {
        printk("device创建失败\r\n");
        goto device_create_faided;
    }
    // printk("device_create\r\n");

    //查找设备节点


    //获得IO编号
    

    //申请IO    
    


    //IO设置


set_gpio_faided:
    gpio_free(key.gpio_num);    //释放IO


find_node_faided:
    device_destroy(key.class,key.devid);         //删除设备

device_create_faided:
    class_destroy(key.class);                   //删除类

class_create_faided:
    cdev_del(&key.cdev);                         //卸载设备 
    

cdev_init_faided:
    unregister_chrdev_region(key.devid,1);      //释放设备号


apply_for_devid_faided:
    return ret;
}

static void __exit alientek_key_exit(void)
{
    //关闭设备
    

    //释放IO
    

    //删除device和class类
    device_destroy(key.class,key.devid);
    class_destroy(key.class);

    //卸载设备
    cdev_del(&key.cdev);

    //释放设备号
    unregister_chrdev_region(key.devid,1);

}


module_init(alientek_key_init);
module_exit(alientek_key_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DYF");