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

#define LED_MAJOR 200
#define LED_NAME "led"


//寄存器的物理地址
#define CCM_CCGR1_BASE              (0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE      (0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE      (0X020E02F4)
#define GPIO1_DR                    (0X0209C000)
#define GPIO1_GDIR                  (0X0209C004)   


//虚拟地址
static void __iomem* IMX6ULL_CCM_CCGR1;
static void __iomem* IMX6ULL_SW_MUX_GPIO1_IO03;
static void __iomem* IMX6ULL_SW_PAD_GPIO1_IO03;
static void __iomem* IMX6ULL_GPIO1_DR;
static void __iomem* IMX6ULL_GPIO1_GDIR;

#define LED_OFF (0)
#define LED_ON  (1)

//设备结构体
static struct chardev_led{
    dev_t devid;    //设备号
    u32 major;      //主设备号
    u32 minor;      //次设备号
    unsigned char dev_num;  //次设备号数量
    struct cdev cdev;
    struct class* class;    //类
    struct device* device;  //设备
}led;



static void led_switch(unsigned char state)
{
    u32 reg = 0;

    if(state==LED_ON)
    {
        // printk("LED_ON\r\n");
        reg = readl(IMX6ULL_GPIO1_DR);
        reg &= ~(0x1<<3);
        writel(reg, IMX6ULL_GPIO1_DR);
    }
    else
    {
        // printk("LED_OFF\r\n");
        reg = readl(IMX6ULL_GPIO1_DR);
        reg |= (0x1<<3);
        writel(reg, IMX6ULL_GPIO1_DR);
    }
}



static int led_open (struct inode *inode, struct file *filp)
{
    filp->private_data = &led;
    return 0;
}

static ssize_t led_write (struct file * filp, const char __user * buf, size_t count, loff_t * ppos)
{
    int retvalue;
    unsigned char databuf[1];
    retvalue = copy_from_user(databuf,buf,count);
    if(retvalue<0)
    {
        printk("kernel write failed");
        return -EFAULT;
    }
    // printk("databuf[0]=%u\r\n",databuf[0]);

    led_switch(databuf[0]);
    
    return 0;
}

static int led_close (struct inode * inode, struct file * filp)
{
    filp->private_data = NULL;
    return 0;
}

static void led_init(void)
{
    u32 reg = 0;
    
    //地址映射
    IMX6ULL_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE,4);
    IMX6ULL_SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE,4);
    IMX6ULL_SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE,4);
    IMX6ULL_GPIO1_DR = ioremap(GPIO1_DR,4);
    IMX6ULL_GPIO1_GDIR = ioremap(GPIO1_GDIR,4);


    //io初始化,寄存器严格读改写
    reg = readl(IMX6ULL_CCM_CCGR1);
    reg |= (0x2<<26);
    writel(reg, IMX6ULL_CCM_CCGR1);

    reg = readl(IMX6ULL_SW_MUX_GPIO1_IO03);
    reg &= ~(0xf);
    reg |= (0x5);
    writel(reg, IMX6ULL_SW_MUX_GPIO1_IO03);

    writel(0X10B0, IMX6ULL_SW_PAD_GPIO1_IO03);

    reg = readl(IMX6ULL_GPIO1_GDIR);
    reg |= (0x1<<3);
    writel(reg, IMX6ULL_GPIO1_GDIR);

    reg = readl(IMX6ULL_GPIO1_DR);
    reg &= ~(0x1<<3);
    writel(reg, IMX6ULL_GPIO1_DR);       //默认打开led
   
}


static const struct file_operations chardev_ops = {
    .owner = THIS_MODULE,
    .write = led_write,
    .open = led_open,
    .release = led_close,
};


static int __init chardev_init(void) 
{
    int retvalue = 0;


    //初始化LED
    led_init();

    led.dev_num = 1;
    
    //设备号申请

    if(led.major)
    {
        led.devid = MKDEV(led.major,0);
        retvalue = register_chrdev_region(led.devid, led.dev_num, LED_NAME);
    }
    else
    {
        retvalue = alloc_chrdev_region(&(led.devid), 0, led.dev_num, LED_NAME);
        led.major = MAJOR(led.devid);
        led.minor = MINOR(led.devid);
    }
    if(retvalue<0)
    {
        printk("chrdev_region failed\r\n");
        goto devid_failed;
    }
    // printk("chrdev_led major = %u, minor = %u\r\n", led.major, led.minor);

    //设备结构体初始化
    led.cdev.owner = THIS_MODULE;
    cdev_init(&(led.cdev),&chardev_ops);

    //向linux注册设备
    retvalue = cdev_add(&led.cdev,led.devid,1);
    if(retvalue<0)
    {
        printk("cdev_add failed\r\n");
        goto cdev_add_failed;
    }

    //设置自动创建设备节点
    led.class = class_create(THIS_MODULE,LED_NAME);
    led.device = device_create(led.class, NULL, led.devid, NULL, LED_NAME);

cdev_add_failed:
    unregister_chrdev_region(led.devid,led.dev_num);


devid_failed:
    return retvalue;   
    


    return 0;
}

static void __exit chardev_exit(void)
{
    unsigned int reg;
    //关闭设备
    reg = readl(IMX6ULL_GPIO1_DR);
    reg |= (0x1<<3);
    writel(reg, IMX6ULL_GPIO1_DR);       //默认打开led

    device_destroy(led.class, led.devid);
    class_destroy(led.class);

    //释放设备号
    unregister_chrdev_region(led.devid,led.dev_num);

    //取消地址映射
    iounmap(IMX6ULL_CCM_CCGR1);
    iounmap(IMX6ULL_SW_MUX_GPIO1_IO03);
    iounmap(IMX6ULL_SW_PAD_GPIO1_IO03);
    iounmap(IMX6ULL_GPIO1_DR);
    iounmap(IMX6ULL_GPIO1_GDIR);

    //卸载设备
    cdev_del(&(led.cdev));

}



module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DTF");