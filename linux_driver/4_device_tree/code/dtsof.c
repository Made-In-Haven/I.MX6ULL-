//测试of函数获取设备树的属性

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


//文件操作函数



/*模块入口*/
static int __init dtsof_init(void)
{

    int ret = 0;
    struct device_node* back_light_p = NULL;
    struct property * prop = NULL;
    const char * str = NULL;
    u32 num = 0;
    u32 *nums;
    u8 i=0;
    
    //找到backlight节点,路径是"/backlight"
    back_light_p = of_find_node_by_path("/backlight");
    if(back_light_p==NULL)
    {
        printk("find backlight node failed\r\n");
        ret = -EINVAL;
        goto find_node_fail;

    }
    //获取compatible属性
    prop = of_find_property(back_light_p,"compatible",NULL);
    if(prop==NULL)
    {
        printk("get backlight node's compatible failed\r\n");
        ret = -EINVAL;
        goto get_property_fail;
    }
    printk("backlight's compatible=%s\r\n",(char*)prop->value);

    //获取status属性
    ret = of_property_read_string(back_light_p,"status",&str);
    if(ret<0)
    {
        printk("get backlight node's status failed\r\n");
        ret = -EINVAL;
        goto get_property_fail;
    }
    printk("backlight's status=%s\r\n",str);

    //获取default-brightness-level
    ret = of_property_read_u32(back_light_p,"default-brightness-level",&num);
    if(ret<0)
    {
        printk("get backlight node's default-brightness-level failed\r\n");
        ret = -EINVAL;
        goto get_property_fail;
    }
    printk("backlight's default-brightness-level=%u\r\n",num);

    //获取brightness-levels
    num = of_property_count_elems_of_size(back_light_p,"brightness-levels",sizeof(u32));

    nums = (u32*)kmalloc(num*sizeof(u32),GFP_KERNEL);       //内核的申请内存函数

    if(nums==NULL)
    {
        printk("Failed to apply for memory\r\n");
        ret = -EINVAL;
        goto get_property_fail;

    }

    ret = of_property_read_u32_array(back_light_p,"brightness-levels",nums,8);
    if(ret<0)
    {
        printk("get backlight node's brightness-levels failed\r\n");
        ret = -EINVAL;
        goto get_property_fail;
    }
    for(;i<num;i++)
    {
        printk("backlight's brightness-levels[%u] = %u\r\n",i,nums[i]);
    }

    kfree(nums);                //内核的释放内存函数
    



get_property_fail:


find_node_fail:

    return ret;


    
    return 0;
}

/*模块出口*/
static void __exit dtsof_exit(void)
{

}




module_init(dtsof_init);
module_exit(dtsof_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DYF");