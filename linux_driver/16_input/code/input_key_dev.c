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
#include <linux/input.h>
#include <linux/workqueue.h>

#define DEV_NAME "key"
#define KEY_PRESS 0
#define KEY_REALEASE 1
#define INPUT_CODE KEY_0



struct dev{
    struct device_node *nd;         //设备树中的设备节点
    spinlock_t spinlock;            //自旋锁
    u32 gpio;                       //GPIO编号
    struct timer_list timer;        //定时器
    u32 irq_number;                 //中断号
    unsigned long flags;            //中断触发方式
    struct work_struct work;        //下半部处理
    atomic_t key_value;             //按键值
    atomic_t press_count;           //按键按下次数
    struct input_dev *input;         

};
static struct dev dev;


/*定时器中断函数*/
static void timer_handler(unsigned long arg)
{
    char state = 0;
    struct dev* dev = (struct dev*)arg;

    state = gpio_get_value(dev->gpio);
    // printk("KEY VALUE: %d\r\n",state);
    atomic_set(&dev->key_value,state);
    if(state==KEY_PRESS)
    {
        atomic_add(1,&dev->press_count);
    }
    input_report_key(dev->input,INPUT_CODE,atomic_read(&dev->key_value));      //上报按键值
    input_sync(dev->input);
    // input_event(dev->input,EV_REP,INPUT_CODE,atomic_read(&dev->press_count));    //上报按下次数
    // input_sync(dev->input);

}

/*下半部处理函数*/
static void work_func(struct work_struct *work) 
{
 	mod_timer(&dev.timer, jiffies + msecs_to_jiffies(10));
}


/*中断处理函数*/
static irqreturn_t key_irq_handler (int irq_num, void *dev)
{
    schedule_work(&((struct dev*)dev)->work);
    return IRQ_HANDLED;
}   

/*中断初始化*/
int key_irq_init(struct device* device)
{
    int ret = 0;
    dev.nd = device->of_node;

    //初始化定时器
    dev.timer.expires = msecs_to_jiffies(10);
    dev.timer.function = timer_handler;
    dev.timer.data = (unsigned long)&dev;
    init_timer(&dev.timer);

    //获取中断号
    dev.irq_number = irq_of_parse_and_map(dev.nd,0);
    if(dev.irq_number<0)
    {
        ret = -ENAVAIL;
        printk("get_irq_number_failed\r\n");
        goto get_irq_number_failed;

    }
    //工作队列初始化
    INIT_WORK(&dev.work, work_func);


    //中断并激活申请
    ret = request_irq(dev.irq_number,key_irq_handler,IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING,"key_irq",&dev);
    if(ret<0)
    {
        printk("request_irq_failed\r\n");
        goto request_irq_failed;

    }


    return 0;


request_irq_failed:


get_irq_number_failed:
    del_timer_sync(&dev.timer);
    return ret;    

}




int input_dev_probe(struct platform_device *platform_device)
{
    int ret = 0;
    struct device device = platform_device->dev;

    //初始化设备
    spin_lock_init(&dev.spinlock);
    dev.nd = device.of_node;
    if(dev.nd==NULL)
    {
        ret = -ENAVAIL;
        printk("of_get_node_failed\r\n");
        goto of_get_node_failed;
    }
    dev.gpio = of_get_named_gpio(dev.nd,"key-gpio",0);
    if(dev.gpio<0)
    {
        ret = -ENAVAIL;
        printk("of_get_gpio_failed\r\n");
        goto of_get_node_failed;
    }
    ret = gpio_request(dev.gpio,"key-gpio");
    if(ret<0)
    {
        printk("gpio_request_failed\r\n");
        goto of_get_node_failed;
    }
    ret = gpio_direction_input(dev.gpio);  
    if(ret<0)
    {
        printk("set_direction_failed\r\n");
        goto set_direction_failed;
    }

    atomic_set(&dev.key_value,0xff);          //初始化一个无效值
    atomic_set(&dev.press_count,0);

    //input_dev初始化
    dev.input = input_allocate_device();
    if(dev.input==NULL)
    {
        ret = -ENAVAIL;
        printk("inputdev_allocate_failed\r\n");
        goto key_irq_init_failed;
    }
    dev.input->name = DEV_NAME;
    __set_bit(EV_KEY,dev.input->evbit);
    // __set_bit(EV_REP,dev.input->evbit);
    __set_bit(KEY_0,dev.input->keybit);
    ret = input_register_device(dev.input);
    if(ret<0)
    {
        printk("input_register_failed\r\n");
        goto input_register_failed;
    }


    //中断和定时器初始化
    ret = key_irq_init(&device);
    if(ret<0)
    {
        printk("key_irq_init_failed\r\n");
        goto key_irq_init_failed;
    }


    return 0;

key_irq_init_failed:
    input_unregister_device(dev.input);

input_register_failed:
    input_free_device(dev.input);

set_direction_failed:
    gpio_free(dev.gpio);

of_get_node_failed:

    return ret;
}

int input_dev_remove(struct platform_device *platform_device)
{

    free_irq(dev.irq_number,&dev);

    del_timer_sync(&dev.timer);

    input_unregister_device(dev.input);

    input_free_device(dev.input);

    gpio_free(dev.gpio);

    return 0;
}



static struct of_device_id dev_match_table[] = {
    {.compatible = "key-alientek"},
    {}
}; 

static struct platform_driver misc_dev_driver = {
    .probe = input_dev_probe,
    .remove = input_dev_remove,
    .driver = {
        .of_match_table = dev_match_table,
        .name = "key"
    }
};



static int __init input_dev_init(void)
{
    return platform_driver_register(&misc_dev_driver);
}

static void __exit input_dev_exit(void)
{
    platform_driver_unregister(&misc_dev_driver);
}

module_init(input_dev_init);
module_exit(input_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DYF");