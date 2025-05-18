# 新字符设备驱动




## 静态申请字符设备号

之前使用`register_chrdev`函数注册设备，浪费了很多次设备号。而且需要手动指定主设备号，容易产生设备号冲突的问题.



## 动态申请字符设备号

**Linux 下每个设备都有一个设备号（不能冲突），linux内核使用dev_t数据类型来定义设备号，设备号分为主设备号和次设备号两部分。dev_t就是一个unsigned int,其中高 12 位为主设备号， 低 20 位为次设备号。** 

我们需要使用`alloc_chrdev_region`函数申请设备号

```c
/* 
 * alloc_chrdev_region() - register a range of char device numbers
 * @dev: output parameter for first assigned number
 * @baseminor: 次设备号从哪个数字开始注册,一般设为0
 * @count: the number of minor numbers required
 * @name: the name of the associated device or driver
 * Returns zero or a negative error code.
*/
    int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,const char *name)
```

使用`unregister_chrdev_region`释放设备号

```c
/**
 * unregister_chrdev_region() - return a range of device numbers
 * @from: the first in the range of numbers to unregister
 * @count: the number of device numbers to unregister
 *
 * This function will unregister a range of @count device numbers,
 * starting with @from.  The caller should normally be the one who
 * allocated those numbers in the first place...
 */
void unregister_chrdev_region(dev_t from, unsigned count)
```



若指定了主设备号，可以使用

```c
//参数 from 是要申请的起始设备号，也就是给定的设备号；参数 count 是要申请的数量，一般都是一个；参数 name 是设备名字。
int register_chrdev_region(dev_t from, unsigned count, const char *name)
```



例子：

```c
	if (major) {					//给定主设备号
		devid = MKDEV(major, 0);	//构建完整设备号
		rc = register_chrdev_region(devid, MAX_PINS, "scx200_gpio");	//申请次设备号
	} else {						//没有给定主设备号
		rc = alloc_chrdev_region(&devid, 0, MAX_PINS, "scx200_gpio");	//申请设备号
		major = MAJOR(devid);		//根据设备号生成主设备号
	}
```





## 字符设备操作函数集注册

### cdev

在 Linux 中使用 **cdev** 结构体表示一个字符设备，` cdev` 结构体在 include/linux/cdev.h 文件中

```c
struct cdev {
	struct kobject kobj;
	struct module *owner;
	const struct file_operations *ops;
	struct list_head list;
	dev_t dev;
	unsigned int count;
};
```

在` cdev `中有两个重要的成员变量： ops 和 dev，这两个就是字符设备文件操作函数集合`file_operations `以及设备号` dev_t`。编写字符设备驱动之前需要定义一个` cdev `结构体变量，这个变量就表示一个字符设备，如下所示：

```c
struct cdev chardev;
```



### cdev_init

先对struct module *owner赋值，例如`newchrled.		cdev.owner = THIS_MODULE;`

就可以使用`cdev_init`函数进行cdev初始化：

```c
void cdev_init(struct cdev *, const struct file_operations *);
```



### cdev_add

`cdev_add` 函数用于向 Linux 系统注册字符设备(cdev 结构体变量)，首先使用` cdev_init `函数完成对 cdev 结构体变量的初始化，然后使用 `cdev_add `函数向 Linux 系统添加这个字符设备。	

```c
int cdev_add(struct cdev *, dev_t, unsigned);
```

- 第一个参数指向要添加的字符设备(cdev 结构体变量)，

- 第二个参数就是设备所使用的设备号，

- 第三个参数是要添加的设备数量。

  

**申请设备号、初始化设备结构体、注册字符设备，这3个步骤就完成了之前`register_chrdev`函数的功能**







## 卸载字符设备



#### cdev_del

卸载驱动的时候一定要使用 cdev_del 函数从 Linux 内核中删除相应的字符设备， cdev_del函数原型如下：

```c
void cdev_del(struct cdev *);
```





## 自动创建设备节点



### mdev

**udev** 是一个用户程序，在 Linux 下通过 udev 来实现设备文件的创建与删除， udev 可以检测系统中硬件设备状态，可以根据系统中硬件设备状态来创建或者删除设备文件。比如使用modprobe 命令成功加载驱动模块以后就自动在/dev 目录下创建对应的设备节点文件,使用rmmod 命令卸载驱动模块以后就删除掉/dev 目录下的设备节点文件。 **使用 busybox 构建根文件系统的时候， busybox 会创建一个 udev 的简化版本—mdev，所以在嵌入式 Linux 中我们使用mdev 来实现设备节点文件的自动创建与删除**， Linux 系统中的热插拔事件也由 mdev 管理，在/etc/init.d/rcS 文件中如下语句：

```shell
echo /sbin/mdev > /proc/sys/kernel/hotplug
```



### 创建和删除class





### 创建和卸载device



参考示例：

```c
//设备结构体
static struct chardev_led{
    dev_t devid;    
    u32 major;      
    u32 minor;      
    unsigned char dev_num;  
    struct cdev cdev;
    struct class* class;    //类
    struct device* device;  //设备
}led;

/* 驱动入口函数 */
static int __init led_init(void)
{				
	/* 创建类 */
	class = class_create(THIS_MODULE, LED_NAME);
 	/* 创建设备 */
 	device = device_create(class, NULL, devid, NULL, LED_NAME);
 	return 0;
}

/* 驱动出口函数 */
static void __exit led_exit(void)
{
 	/* 删除设备 */
 	device_destroy(newchrled.class, newchrled.devid);
 	/* 删除类 */
 	class_destroy(newchrled.class);
}

```







## 设置文件私有属性

```c
/* open 函数 */
static int test_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &testdev; /* 设置私有数据 */
	return 0;
}
```

在open时，设置文件私有属性，之后read、write就可以直接访问文件的私有属性。





## 错误处理

使用goto进行错误处理，或者写一个错误处理函数

例如申请设备号成功，但向linux中注册设备cdev_add失败，**==这时就需要把申请到的设备号释放掉==**

例如：

```c
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
if(retvalue<0)				//设备号申请失败
{
    printk("chrdev_region failed\r\n");
    goto devid_failed;
}

retvalue = cdev_add(&led.cdev,led.devid,1);
if(retvalue<0)				//设备注册失败
{
    printk("cdev_add failed\r\n");
    goto cdev_add_failed;
}


cdev_add_failed:
    unregister_chrdev_region(led.devid,led.dev_num)		//释放已经申请的设备号

devid_failed:
    return retvalue;   
    

```







## 总结

**`init`函数中需要完成：设备初始化（可选）、申请字符设备号、初始化字符设备结构体`cdev`、向linux注册字符设备**



**`exit`函数需要完成：关闭设备（可选），释放字符设备号、卸载字符设备**
