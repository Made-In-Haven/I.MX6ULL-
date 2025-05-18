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

#define LED_MAJOR 200
#define LED_NAME "led"


//寄存器的物理地址
#define CCM_CCGR1_BASE 			    (0X020C406C)
#define SW_MUX_GPIO1_IO03_BASE      (0X020E0068)
#define SW_PAD_GPIO1_IO03_BASE      (0X020E02F4)
#define GPIO1_DR 			        (0X0209C000)
#define GPIO1_GDIR 			        (0X0209C004)   

//虚拟地址
static void __iomem* IMX6ULL_CCM_CCGR1;
static void __iomem* IMX6ULL_SW_MUX_GPIO1_IO03;
static void __iomem* IMX6ULL_SW_PAD_GPIO1_IO03;
static void __iomem* IMX6ULL_GPIO1_DR;
static void __iomem* IMX6ULL_GPIO1_GDIR;

#define LED_OFF (0)
#define LED_ON  (1)


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
    return 0;
}



/*字符设备操作集合*/
static struct file_operations led_operations = {
    .owner		= THIS_MODULE,
	.write		= led_write,
	.open		= led_open,
	.release	= led_close,
};

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

/*驱动入口函数*/
static int __init led_dev_init(void)
{
    int ret;

    //初始化led
    led_init();
    //注册字符设备设备号
    ret = register_chrdev(LED_MAJOR,LED_NAME,&led_operations);
    if(ret<0)
    {
        printk(KERN_WARNING LED_NAME": could not get major number\n");
        return -EIO;
    }

    
    printk("led_dev init\r\n");
    return 0;
}


/*驱动出口函数*/
static void __exit led_dev_exit(void)
{
    unsigned int reg = 0;
    //关灯
    reg = readl(IMX6ULL_GPIO1_DR);
    reg |= (0x1<<3);
    writel(reg, IMX6ULL_GPIO1_DR);


    //取消地址映射
    iounmap(IMX6ULL_CCM_CCGR1);
    iounmap(IMX6ULL_SW_MUX_GPIO1_IO03);
    iounmap(IMX6ULL_SW_PAD_GPIO1_IO03);
    iounmap(IMX6ULL_GPIO1_DR);
    iounmap(IMX6ULL_GPIO1_GDIR);

    //卸载字符设备设备号
    unregister_chrdev(LED_MAJOR,LED_NAME);
    
    printk("led_dev exit\r\n");
}


/*驱动加载和卸载*/
module_init(led_dev_init);
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DYF");
