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
#include <linux/interrupt.h>

#define KEY_NAME "key"
#define KEY_ON 1
#define KEY_OFF 0
#define KEY_NUM 1           //按键数量
#define KEY_INVALID 0XFF    //按键无效


//按键结构体
struct irq_key{
    int gpio;       //按键对应的io编号
    int irqnum;    //按键io对应的中断号
    
    char name[10];  //按键名字
    unsigned long flags;    //中断触发方式
    irq_handler_t key_irqhandler;                //按键对应的中断处理函数
    struct tasklet_struct tasklet;      //tasklet
    void (*tasklet_func)(unsigned long);
    struct timer_list timer;
    atomic_t value;         //按键值
    atomic_t press_time;    //按下按键的次数
    struct fasync_struct* fasync;    //异步通知结构体
};

/*设备结构体*/
static struct dev
{
    dev_t devid;
    u32 major;
    u32 minor;
    struct cdev cdev;
    struct device* device;
    struct class* class;
    struct device_node *nd;
    struct irq_key key[KEY_NUM];
    
}dev;



/*写入函数*/
static ssize_t dev_write (struct file * filp, const char __user * buf, size_t count, loff_t * ppos)
{

    
    return count;
}
/*读取函数*/
static int dev_open (struct inode * inode, struct file * filp)
{
    filp->private_data = &dev;
    return 0;
}


/*读取函数*/
ssize_t dev_read (struct file * filp, char __user * buf, size_t count, loff_t * ppos)
{
    int ret = 0;    
    unsigned char databuf[2];
    struct irq_key * pkey0 = &((struct dev*)filp->private_data)->key[0];

    databuf[0] = atomic_read(&pkey0->value);         //按键值
    databuf[1] = atomic_read(&pkey0->press_time);    //按下次数


    ret = copy_to_user(buf,databuf,sizeof(databuf));
    if(ret<0)
    {
        printk("data error\r\n");
        return ret;
    }

    return count;

}

int dev_fasync (int fd, struct file * filp, int on)
{
    struct dev* dev = (struct dev*)filp->private_data;
    return fasync_helper(fd,filp,on,&dev->key[0].fasync);
   
}

/*释放函数*/
static int dev_release (struct inode * inode, struct file * filp)
{
    
    return dev_fasync(-1,filp,0);
}


/*设备操作结构体*/
static const struct file_operations dev_opts = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .write = dev_write,
    .release = dev_release,
    .read = dev_read,
    .fasync = dev_fasync,
};

/*按键1定时器处理函数
* 消抖结束进入处理函数
*/
void key1_timer_func(unsigned long arg)
{
    static unsigned int count = 0;
    unsigned char key_state = 0;
    struct irq_key * irq_key0 = (struct irq_key *)arg;

    key_state = gpio_get_value(irq_key0->gpio);
    atomic_set(&irq_key0->value,key_state);
    


    if(key_state==0)
    {
        // printk("press key\r\n");
        count++;
        atomic_set(&irq_key0->press_time,count);

    }
    else
    {
        // printk("free key\r\n");
    }
    kill_fasync(&irq_key0->fasync,SIGIO,POLL_IN);   //向应用程序发信号，设备可读
}

//按键中断处理函数
irqreturn_t key0_handler_t(int irq, void * data_p)
{
    struct irq_key* irq_key0 = data_p;

    //初始化并定时器
    mod_timer(&irq_key0->timer,jiffies + msecs_to_jiffies(10));



    // printk("key0_handler_t\r\n");

    // tasklet_schedule(&(irq_key0->tasklet));
    return 0;
}

irqreturn_t key1_handler_t(int irq, void * data_p)
{
    return IRQ_HANDLED;
}

irqreturn_t key2_handler_t(int irq, void * data_p)
{
    return IRQ_HANDLED;
}

//tasklet处理函数
void key0_tasklet_func(unsigned long data)
{
    printk("key0_tasklet_func\r\n");
}
void key1_tasklet_func(unsigned long data)
{
    
}
void key2_tasklet_func(unsigned long data)
{
    
}


/*按键io初始化函数*/
static int keyio_init(struct dev* dev)
{
    int ret=0;
    int i=0;
    int j=0;

    for(;i<KEY_NUM;i++)
    {
        //获取设备节点
        dev->nd = of_find_node_by_path("/key");
        if(dev->nd==NULL)
        {
            printk("%d_find_node_failed\r\n",i);
            ret = -EINVAL;
            goto find_node_failed;
        }
        //获取IO编号
        dev->key[i].gpio = of_get_named_gpio(dev->nd,"key-gpio",i);
        if(dev->key[i].gpio<0)
        {
            printk("%d_of_get_named_gpio_failed\r\n",i);
            ret = -EINVAL;
            goto get_named_gpio_failed;
        }

        //申请IO
        memset(dev->key[i].name,0,sizeof(dev->key[i].name));        //清空名字字符串
        sprintf(dev->key[i].name,"key%d",i);                      //设置key名称为keyi
        ret = gpio_request(dev->key[i].gpio,dev->key[i].name);
        if(ret<0)
        {
            printk("%d_gpio_request_failed\r\n",i);
            goto find_node_failed;
        }

        //设置IO为输入
        ret = gpio_direction_input(dev->key[i].gpio);
        if(ret<0)
        {
            printk("%d_gpio_direction_input_failed\r\n",i);
            goto gpio_direction_input_failed;
        }
    }
    return 0;

get_named_gpio_failed:          //当前io申请失败，释放之前申请到的IO
    for(;j<i;j++)
    {
        gpio_free(dev->key[i].gpio);
    }

gpio_direction_input_failed:    //如果设置失败，则释放所有申请到的IO
    for(;j<=i;j++)
    {
        gpio_free(dev->key[i].gpio);
    }
    

find_node_failed:


    return ret;

}


/*按键中断初始化函数*/
static int keyirq_init(struct dev* dev)
{
    int i = 0;
    int j = 0;
    int ret = 0;

    //初始化按键中断的上半部和下半部处理函数，这里只有一个按键，所以只初始化了key[0]
    dev->key[0].key_irqhandler = key0_handler_t;
    dev->key[0].tasklet_func = key0_tasklet_func;

    for(i=0;i<KEY_NUM;i++)
    {
        //设置所有按键都是跳变沿触发
        dev->key[i].flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING; 
    }

    for(i=0;i<KEY_NUM;i++)
    {
        //从设备树中获取中断号
        dev->key[i].irqnum = irq_of_parse_and_map(dev->nd,0);
        // printk("key0 irqnum:%d\r\n",dev->key[i].irqnum);
        // printk("key0 io号:%d\r\n",dev->key[i].gpio);

        //初始化tasklet
        tasklet_init(&(dev->key[i].tasklet),dev->key[i].tasklet_func,0);
        
        //申请并激活中断
        ret = request_irq(dev->key[i].irqnum,dev->key[i].key_irqhandler,
                            dev->key[i].flags,dev->key[i].name,&(dev->key[i]));
        if(ret<0)
        {
            printk("%d_request_irq_failed",i);
            goto request_irq_failed;
        }

    }

    return 0;

request_irq_failed:         //当前按键中断申请失败，释放之前申请到的按键中断
    for(j=0;j<i;j++)
    {
        gpio_free(dev->key[i].gpio);
    }



    return ret;
}



/*设备初始化函数*/
static int __init alientek_dev_init(void)
{
    int ret = 0;
    int i = 0;


    //申请设备号
    ret = alloc_chrdev_region(&(dev.devid),0,1,KEY_NAME);
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
    if(ret<0)
    {
        printk("cdev_add faided\r\n");
        goto cdev_init_faided;
    }

    //初始化device和class

    dev.class = class_create(THIS_MODULE,KEY_NAME);
    if(dev.class==NULL)
    {
        printk("class创建失败\r\n");
        goto class_create_faided;
    }
    // printk("class_create\r\n");
    dev.device = device_create(dev.class, NULL, dev.devid, NULL, KEY_NAME);
    if(dev.device==NULL)
    {
        printk("device创建失败\r\n");
        goto device_create_faided;
    }
    printk("device_create\r\n");

    //设备初始化
    ret = keyio_init(&dev);
    if(ret<0)
    {
        printk("keyio_init_failed\r\n");
        goto find_node_faided;
    }
    printk("keyio_init 成功\r\n");

    //中断初始化

    ret = keyirq_init(&dev);
    if(ret<0)
    {
        printk("keyirq_init_failed\r\n");
        goto keyirq_init_failed;
    }
    printk("keyirq_init 成功\r\n");

    //定时器初始化
    init_timer(&dev.key[0].timer);
    dev.key[0].timer.function = key1_timer_func;
    dev.key[0].timer.data = (unsigned long)(&dev.key[0]);

    //按键原子变量初始化
    atomic_set(&dev.key[0].value, KEY_INVALID);  //按键状态初始值为无效
    atomic_set(&dev.key[0].press_time, 0);       //按键按下次数


    return 0;


keyirq_init_failed:
    for(i=0;i<KEY_NUM;i++)
    {
        gpio_free(dev.key[i].gpio);        //释放IO
    }


find_node_faided:
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
    int i = 0;
    //关闭设备


    //关闭中断

    for(i = 0;i<KEY_NUM;i++)
    {
        free_irq(dev.key[i].irqnum,&(dev.key[i]));
        gpio_free(dev.key[i].gpio);
    }
    // free_irq(dev.key[0].irqnum,&dev);
    // gpio_free(dev.key[0].gpio);

    //删除定时器
    del_timer_sync(&dev.key[0].timer);
   

    //删除device和class类
    device_destroy(dev.class,dev.devid);
    class_destroy(dev.class);

    //卸载设备
    cdev_del(&dev.cdev);

    //释放设备号
    unregister_chrdev_region(dev.devid,1);

}


module_init(alientek_dev_init);
module_exit(alientek_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DYF");