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

#define BEEP_NAME "beep"
#define BEEP_ON 1
#define BEEP_OFF 0




static struct pinctl_beep_dev
{
    dev_t devid;
    u32 major;
    u32 minor;
    struct cdev cdev;
    struct device* device;
    struct class* class;
    struct device_node *nd;
    u32 gpio_num;

    
}beep;

void beep_switch(unsigned char state,struct file * filp)
{
    // printk("beep_switch\r\n");
    switch (state)
    {
    case BEEP_ON:
        /* code */
        gpio_set_value(((struct pinctl_beep_dev*)(filp->private_data))->gpio_num,0);
        break;

    case BEEP_OFF:
        gpio_set_value(((struct pinctl_beep_dev*)(filp->private_data))->gpio_num,1);
        break;
    
    default:
        gpio_set_value(((struct pinctl_beep_dev*)(filp->private_data))->gpio_num,1);
        break;
    }
}


ssize_t beep_write (struct file * filp, const char __user * buf, size_t count, loff_t * ppos)
{
    unsigned char databuf[1];
    if (copy_from_user(databuf, buf, sizeof(databuf)))
        return -EFAULT;
    if(databuf[0]!='0'&&databuf[0]!='1')
    {
        printk("\r\n");
        return -EINVAL;
    }
    beep_switch(databuf[0]-'0',filp);
    
    return count;
}
int beep_open (struct inode * inode, struct file * filp)
{
    filp->private_data = &beep;
    return 0;
}
int beep_release (struct inode * inode, struct file * filp)
{
    
    return 0;
}



static const struct file_operations beep_opts = {
    .owner = THIS_MODULE,
    .open = beep_open,
    .write = beep_write,
    .release = beep_release,
};




static int __init beep_init(void)
{
    int ret = 0;


    //申请设备号
    ret = alloc_chrdev_region(&(beep.devid),0,1,BEEP_NAME);
    if(ret<0)
    {
        printk("申请设备号失败\r\n");
        goto apply_for_devid_faided;
    }

    beep.major = MAJOR(beep.devid);
    beep.minor = MINOR(beep.devid);
    // printk("beep major = %u\r\n",beep.major);


    //设备结构体初始化
    beep.cdev.owner = THIS_MODULE;
    cdev_init(&beep.cdev,&beep_opts);

    //向linux注册设备
    ret = cdev_add(&beep.cdev,beep.devid,1);
    if(ret<0)
    {
        printk("cdev_add faided\r\n");
        goto cdev_init_faided;
    }

    //初始化device和class

    beep.class = class_create(THIS_MODULE,BEEP_NAME);
    if(beep.class==NULL)
    {
        printk("class创建失败\r\n");
        goto class_create_faided;
    }
    // printk("class_create\r\n");
    beep.device = device_create(beep.class, NULL, beep.devid, NULL, BEEP_NAME);
    if(beep.device==NULL)
    {
        printk("device创建失败\r\n");
        goto device_create_faided;
    }
    // printk("device_create\r\n");

    //查找设备节点
    beep.nd = of_find_node_by_path("/beep");      
    if(beep.nd==NULL)
    {
        printk("设备节点查找失败\r\n");
        goto find_node_faided;
    }

    //获得IO编号
    beep.gpio_num = of_get_named_gpio(beep.nd,"beep-gpio",0);  
    if(beep.gpio_num<0)
    {
        printk("beep-gpio查找失败\r\n");
        goto find_node_faided;
    }
    printk("beep-gpio number:%u\r\n",beep.gpio_num);


    //申请IO    
    ret = gpio_request(beep.gpio_num,"beep-gpio");    
    if(ret)
    {
        printk("faided to request GPIO for beep \r\n");
        goto find_node_faided;
    }


    //IO设置
    ret = gpio_direction_output(beep.gpio_num,1);    //默认输出高电平，beep灭
    if(ret)
    {
        printk("faided to set GPIO for beep \r\n");
        goto set_gpio_faided;
    }

    return 0;

set_gpio_faided:
    gpio_free(beep.gpio_num);    //释放IO


find_node_faided:
    device_destroy(beep.class,beep.devid);         //删除设备

device_create_faided:
    class_destroy(beep.class);                   //删除类

class_create_faided:
    cdev_del(&beep.cdev);                         //卸载设备 
    

cdev_init_faided:
    unregister_chrdev_region(beep.devid,1);      //释放设备号


apply_for_devid_faided:
    return ret;
}

static void __exit beep_exit(void)
{
    //关闭设备
    gpio_set_value(beep.gpio_num,1);

    //释放IO
    gpio_free(beep.gpio_num);

    //删除device和class类
    device_destroy(beep.class,beep.devid);
    class_destroy(beep.class);

    //卸载设备
    cdev_del(&beep.cdev);

    //释放设备号
    unregister_chrdev_region(beep.devid,1);

}


module_init(beep_init);
module_exit(beep_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DYF");