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

#define LED_MAJOR 200
#define LED_NAME "led"

#define LED_OFF (0)
#define LED_ON  (1)


//物理地址
u32 CCM_CCGR1_BASE;
u32 SW_MUX_GPIO1_IO03_BASE;
u32 SW_PAD_GPIO1_IO03_BASE;
u32 GPIO1_DR;
u32 GPIO1_GDIR;    



//设备私有结构体

static struct led_dev{
    dev_t devid;
    u32 major;
    u32 minor;
    struct cdev cdev;
    struct class* class;
    struct device* device;
    struct device_node* node;

    //虚拟地址
    void __iomem* IMX6ULL_CCM_CCGR1;
    void __iomem* IMX6ULL_SW_MUX_GPIO1_IO03;
    void __iomem* IMX6ULL_SW_PAD_GPIO1_IO03;
    void __iomem* IMX6ULL_GPIO1_DR;
    void __iomem* IMX6ULL_GPIO1_GDIR;

}led; 


static int led_open (struct inode * inode, struct file * filp)
{
    filp->private_data = &led;
    return 0;
}

/*
filp：file结构体指针
buf：应用程序写入的数据
count: 写入的字节数
ppos：相对于文件首地址的偏移
*/

static ssize_t led_write (struct file *filp, const char __user * buf, size_t count, loff_t * ppos)
{
    
    
    return 0;
}
static int led_close (struct inode * inode, struct file * filp)
{
    return 0;
}



static struct file_operations led_opts = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
    .release = led_close
};


void led_init(void)
{
    u32 reg = 0;
    
    //读取设备树reg属性
#if 0
    of_property_read_u32_index(led.node,"reg",0,&CCM_CCGR1_BASE);
    of_property_read_u32_index(led.node,"reg",2,&SW_MUX_GPIO1_IO03_BASE);
    of_property_read_u32_index(led.node,"reg",4,&SW_PAD_GPIO1_IO03_BASE);
    of_property_read_u32_index(led.node,"reg",6,&GPIO1_DR);
    of_property_read_u32_index(led.node,"reg",8,&GPIO1_GDIR);

    // printk("GPIO1_GDIR: %x\r\n",GPIO1_GDIR);
    // printk("GPIO1_GDIR: %x\r\n",SW_PAD_GPIO1_IO03_BASE);

    //内存映射
    led.IMX6ULL_CCM_CCGR1 = ioremap(CCM_CCGR1_BASE,0x04);
    led.IMX6ULL_SW_MUX_GPIO1_IO03 = ioremap(SW_MUX_GPIO1_IO03_BASE,0x04);
    led.IMX6ULL_SW_PAD_GPIO1_IO03 = ioremap(SW_PAD_GPIO1_IO03_BASE,0x04);
    led.IMX6ULL_GPIO1_DR = ioremap(GPIO1_DR,0x04);
    led.IMX6ULL_GPIO1_GDIR = ioremap(GPIO1_GDIR,0x04);
#endif

    led.IMX6ULL_CCM_CCGR1 = of_iomap(led.node,0);
    led.IMX6ULL_SW_MUX_GPIO1_IO03 = of_iomap(led.node,1);
    led.IMX6ULL_SW_PAD_GPIO1_IO03 = of_iomap(led.node,2);
    led.IMX6ULL_GPIO1_DR = of_iomap(led.node,3);
    led.IMX6ULL_GPIO1_GDIR = of_iomap(led.node,4);

    //初始化
    reg = readl(led.IMX6ULL_CCM_CCGR1);
    reg |= (0x2<<26);
    writel(reg, led.IMX6ULL_CCM_CCGR1);

    reg = readl(led.IMX6ULL_SW_MUX_GPIO1_IO03);
    reg &= ~(0xf);
    reg |= (0x5);
    writel(reg, led.IMX6ULL_SW_MUX_GPIO1_IO03);

    writel(0X10B0, led.IMX6ULL_SW_PAD_GPIO1_IO03);

    reg = readl(led.IMX6ULL_GPIO1_GDIR);
    reg |= (0x1<<3);
    writel(reg, led.IMX6ULL_GPIO1_GDIR);

    reg = readl(led.IMX6ULL_GPIO1_DR);
    reg &= ~(0x1<<3);
    writel(reg, led.IMX6ULL_GPIO1_DR);       //默认打开led





    
    
    
    
    // ioremap();
}


static int __init dtsled_init(void)
{
    int ret = 0;
    //设备初始化

    
    //申请设备号
    ret = alloc_chrdev_region(&led.devid,0,1,LED_NAME);
    if(ret<0)
    {
        printk("申请设备号失败\r\n");
        goto apply_for_devid_failed;
    }
    led.major = MAJOR(led.devid);
    led.minor = MINOR(led.devid);



    //向Linux中注册设备
    led.cdev.owner = THIS_MODULE;
    cdev_init(&led.cdev,&led_opts);

    ret = cdev_add(&led.cdev,led.devid,1);
    if(ret<0)
    {
        printk("注册设备失败\r\n");
        goto cdev_add_failed;
    }
    

    //创建类和设备
    led.class = class_create(THIS_MODULE,LED_NAME);
    led.device = device_create(led.class,NULL,led.devid,NULL,LED_NAME);

    led.node = of_find_node_by_path("/led");
    if(led.node==NULL)
    {
        printk("查找节点失败\r\n");
        goto find_node_failed;
    }

    led_init();
    return 0;


find_node_failed:
    device_destroy(led.class,led.devid);
    class_destroy(led.class);
    cdev_del(&led.cdev);



cdev_add_failed:
    unregister_chrdev_region(led.devid,1);


apply_for_devid_failed:

    return ret;



   
}

static void __exit dtsled_exit(void)
{   
    u32 reg = 0;
    //关闭设备
    reg = readl(led.IMX6ULL_GPIO1_DR);
    reg |= (0x1<<3);
    writel(reg, led.IMX6ULL_GPIO1_DR);       
    
    //删除类和设备
    device_destroy(led.class,led.devid);
    class_destroy(led.class);

    //在linux中注销设备
    cdev_del(&led.cdev);

    //释放设备号
    unregister_chrdev_region(led.devid,1);




    //取消内存映射
    iounmap(led.IMX6ULL_CCM_CCGR1);
    iounmap(led.IMX6ULL_SW_MUX_GPIO1_IO03);
    iounmap(led.IMX6ULL_SW_PAD_GPIO1_IO03);
    iounmap(led.IMX6ULL_GPIO1_DR);
    iounmap(led.IMX6ULL_GPIO1_GDIR);



}


module_init(dtsled_init);
module_exit(dtsled_exit);

MODULE_AUTHOR("DYF");
MODULE_LICENSE("GPL");