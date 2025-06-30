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
#define KEY_VALID 0
#define KEY_INVALID 1




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
    atomic_t key_value;

    
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
static ssize_t key_read (struct file *filp, char __user *buf, size_t count, loff_t * ppos)
{
    int ret;
    struct key_dev * dev = filp->private_data;
    char databuf[1];
    if(gpio_get_value(dev->gpio_num)==0)
    {
        while(gpio_get_value(dev->gpio_num))
        {
            atomic_set(&dev->key_value,KEY_VALID);
        }
    }
    else
    {
        atomic_set(&(dev->key_value),KEY_INVALID);
    }
    databuf[0] = (char)atomic_read(&dev->key_value);


    ret = copy_to_user(buf,databuf,1);
    if(ret<0)
    {
        printk("内存拷贝失败\r\n");
        return -EINVAL;
    }
    return count;
}



static const struct file_operations key_opts = {
    .owner = THIS_MODULE,
    .open = key_open,
    .write = key_write,
    .release = key_release,
    .read = key_read
};

/*keyIO初始化函数*/
static int keyio_init(void)
{
    int ret;
    //获取设备节点
    key.nd = of_find_node_by_path("/key");
    if(key.nd==NULL)
    {
        printk("获取设备节点失败\r\n");
        ret = -EINVAL;
        goto find_node_failed;
    }
    //获取IO号
    key.gpio_num = of_get_named_gpio(key.nd,"key-gpio",0);
    printk("获取IO号:%d\r\n",key.gpio_num);
    if(key.gpio_num<0)
    {
        printk("获取IO号失败\r\n");
        ret = -EINVAL;
        goto get_ionum_failed;
    }

    //申请IO
    ret = gpio_request(key.gpio_num,"key0");
    if(key.gpio_num<0)
    {
        printk("申请IO号: %d失败\r\n",key.gpio_num);
        ret = -EBUSY;
        goto gpio_request_failed;
    }

    //设置IO为输入
    ret = gpio_direction_input(key.gpio_num);
    if(ret<0)
    {
        printk("设置IO为输入失败\r\n");
        ret = -EINVAL;
        goto set_input_failed;
    }


    return 0;
    

set_input_failed:
    gpio_free(key.gpio_num);            //释放IO

gpio_request_failed:


get_ionum_failed:


find_node_failed:


    return ret;

}




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
    key.device = device_create(key.class, NULL, key.devid, NULL, KEY_NAME);
    if(key.device==NULL)
    {
        printk("device创建失败\r\n");
        goto device_create_faided;
    }

    /*GPIO初始化*/
    ret = keyio_init();
    if(ret<0)
    {
        printk("gpio初始化失败\r\n");
        goto find_node_faided;
    }
    atomic_set(&key.key_value,KEY_INVALID);


    return 0;



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
    gpio_free(key.gpio_num);

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