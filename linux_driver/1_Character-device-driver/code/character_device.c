#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#define CHARDEV_MAJOR_ID  200
#define CHARDEV_NAME "chardevbase"

static char readbuf[100];	//读缓冲
static char writebuf[100];	//写缓冲
static char kerneldata[] = "kernel data";


static int strdev_open (struct inode *inode, struct file *filp)
{
	// printk("strdev_open\r\n");
	return 0;
}

/*
@description 从设备读取数据
@parameter - filp : 要打开的设备文件
@parameter - buf  : 返回给用户空间的数据缓冲区
@parameter - count: 要读的字节数
@parameter - ppos : 相对于文件首地址的偏移
*/
static ssize_t strdev_read(struct file *filp, char __user *buf,
							size_t count, loff_t *ppos)
{
	
	int ret = 0;
	// printk("strdev_read\r\n");
	memcpy(readbuf,kerneldata,sizeof(kerneldata));
	ret = copy_to_user(buf, readbuf, count);
	if(ret==0)
	{
		return count;
	}
	return 0;
}

/*
@description 向设备中写数据
@parameter - filp : 要打开的设备文件
@parameter - buf  : 要写入设备的用户空间的数据缓冲区
@parameter - count: 要写的字节数
@parameter - ppos : 相对于文件首地址的偏移
*/
static ssize_t strdev_write(struct file *filp, const char __user *buf,
	size_t count, loff_t *ppos)
{
	
	int ret = 0;
	// printk("strdev_write\r\n");
	ret = copy_from_user(writebuf,buf,count);
	
	if(ret == 0)
	{
		// printk("kernel write success\r\n");
	}

	return 0;
}



static int strdev_close(struct inode *inode, struct file *filp)
{
	// printk("strdev_close\r\n");
	return 0;
}

/*字符设备操作集合*/
static struct file_operations test_fops = {
	.owner = THIS_MODULE,
	.open = strdev_open,
	.write = strdev_write,
	.read = strdev_read,
	.release = strdev_close,
};





/*
驱动入口函数
*/
static int __init character_device_init(void)
{
	int retvalue;
	
	/* 入口函数具体内容 */
	printk("character_device_init\n");
	
	retvalue = 0;

	/* 注册字符设备 */

	retvalue = register_chrdev(CHARDEV_MAJOR_ID, CHARDEV_NAME, &test_fops);
	if(retvalue < 0){
		/* 字符设备注册失败,自行处理 */
		printk("字符设备注册失败\n");
		return retvalue;
	}

	return 0;
}


 /* 驱动出口函数 */
static void __exit character_device_exit(void)
{
 	/* 出口函数具体内容 */
	printk(KERN_INFO"character_device_exit\n");

	/*注销字符设备号*/
	unregister_chrdev(CHARDEV_MAJOR_ID, CHARDEV_NAME);
}



module_init(character_device_init);
module_exit(character_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DYF");


