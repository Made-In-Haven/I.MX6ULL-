# Linux中断



## 一、裸机中断处理流程

在裸机系统中，中断处理需要手动完成全流程控制，步骤如下：

1. 编写中断向量表：在启动文件（如start.s）中定义中断向量表，实现 IRQ 中断处理程序的框架（包括现场保护、中断号获取、跳转至通用处理函数）。
2. 使能中断与寄存器初始化：配置中断控制器（如 GIC）、外设中断使能寄存器，开启目标中断。
3. 注册中断处理函数：为特定中断号绑定对应的处理函数，实现具体中断逻辑。
4. 初始化外设中断：配置外设的中断触发方式（如边沿触发 / 电平触发）、中断掩码等参数。








## 二、Linux中断API函数

Linux 内核已封装中断控制器初始化逻辑，开发者只需调用 API 完成中断申请与处理，核心函数如下：



### 1.申请中断函数

==**request_irq 函数用于申请并激活中断**==，request_irq函数可能会**引起睡眠**，因此**不能在中断或其他禁止睡眠的代码段（例如抢占了自旋锁或信号量的线程）**中申请。

```c
int request_irq(unsigned int irq,
	irq_handler_t handler,
	unsigned long flags,
	const char *name,c
	void *dev)
--irq:		要申请的中断号
--handler:	中断处理函数
--name:		中断名字，设置以后可以在/proc/interrupts 文件中看到对应的中断名字。

--flags:	中断标志，多个中断标志可以使用“|”来组合使用。可以在文件 include/linux/interrupt.h 里面查看所有的中断标志。下面列举一些常用的中断标志：
    			IRQF_SHARED:多个设备共享一个中断线，共享的所有中断都必须指定此标志。如果使用共享中断的话，request_irq函数的 dev 参数就是唯一区分他们的标志
                IRQF_ONESHOT：单次中断，执行一次就结束
                IRQF_TRIGGER_RISING：上升沿触发
                IRQF_TRIGGER_FALLING：下降沿触发
                IRQF_TRIGGER_HIGH：高电平触发
                IRQF_TRIGGER_LOW：低电平触发
                    
 
--dev:		如果将 flags 设置为 IRQF_SHARED 的话， dev 用来区分不同的中断，一般情况下将dev 设置			为设备结构体， dev 会传递给中断处理函数 irq_handler_t 的第二个参数。

返回值为0表示申请成功，为负数表示申请失败，返回-EBUSY表示中断已被申请
```





### 2. 释放中断函数

```c
void free_irq(unsigned int irq,
				void *dev)
```

==**！！！注意，这里的dev参数一定要与申请中断函数中的参数保持一致**==



### 3. 中断处理函数

```c
irqreturn_t (*irq_handler_t) (int, void *)

第一个参数是中断号
第二个参数只有在使用共享中断时才有用，若使用共享中断，第二个参数就是共享设备的指针
```

中断处理函数的返回值为 irqreturn_t 类型， irqreturn_t 类型定义如下所示

```c
enum irqreturn {
    IRQ_NONE = (0 << 0),
    IRQ_HANDLED = (1 << 0),
    IRQ_WAKE_THREAD = (1 << 1),
};

typedef enum irqreturn irqreturn_t;
```
返回值类型：irqreturn_t（枚举类型），常用返回值：
- IRQ_HANDLED：表示中断已被正确处理。
- IRQ_NONE：表示未处理该中断（通常用于共享中断）。
- 实际使用时推荐通过宏封装：return IRQ_RETVAL(IRQ_HANDLED);。



### 4. 中断使能和禁止函数
（1）**用于已经申请过的特定中断**
```c
void enable_irq(unsigned int irq)
void disable_irq(unsigned int irq)			//需要等待当前中断执行完成后禁止中断
void disable_irq_nosync(unsigned int irq)	//不需要等待
```



（2）全局中断使能和禁止函数

```c
local_irq_enable()			//全局使能
local_irq_disable()			//全局禁止
    
local_irq_save(flags)		//记录当前处理器上的中断状态（开或关），并保存状态至flags
local_irq_restore(flags)	//根据flags来判断是打开还是关闭中断
```





## 三、上半部和下半部

为平衡中断响应速度与处理复杂度，Linux 将中断处理分为上半部（Top Half）和下半部（Bottom Half）：

**上半部：即中断处理函数，负责快速处理紧急任务（如清除中断标志、记录硬件状态），必须原子执行（不睡眠、不阻塞）。**
**下半部：延迟执行耗时操作（如数据处理、I/O 交互），可在中断上下文之外执行。**



至于哪些代码属于上半部，哪些代码属于下半部并没有明确的规定，一切根据实际使用情况去判断，这个就很考验驱动编写人员的功底了。这里有一些可以借鉴的参考点：

- 如果要处理的内容不希望被其他中断打断，那么可以放到上半部。
- 如果要处理的任务对时间敏感，可以放到上半部。
- 如果要处理的任务与硬件有关，可以放到上半部
- 除了上述三点以外的其他任务，优先考虑放到下半部。



### 1. 下半部处理机制对比


|特性	|软中断（不建议使用）|	Tasklet|	工作队列（Workqueue）|
|-----|----|----|----|
|运行上下文	|软中断上下文|	软中断上下文|	进程上下文（内核线程）|
|睡眠支持	|不允许	|不允许	|允许（可调用msleep）|
|并发控制|	多 CPU 可并行执行|	同一 CPU 串行执行|	同队列串行执行|
适用场景|	高性能需求（如网络）|	简单异步任务|	复杂阻塞任务|
核心 API|	raise_softirq|	tasklet_schedule	|schedule_work|

### 2. 常用下半部实现方式

#### （1）软中断（不建议使用）

软中断在多核场景下遵守 **“谁触发，谁执行”** 原则

Linux 内核使用结构体 softirq_action 表示软中断， softirq_action结构体定义在文件include/linux/interrupt.h 中，内容如下;

```c
struct softirq_action
{
	void (*action)(struct softirq_action *);	//action 成员变量就是软中断的服务函数
};
```

在 kernel/softirq.c 文件中一共定义了 10 个软中断，如下所示

```c
/*数组 softirq_vec 是个全局数组，因此所有的 CPU(对于 SMP 系统而言)都可以访问到，每个 CPU 都有自己的触发和控制机制，并且只执行自己所触发的软中断。但是各个CPU所执行的软中断服务函数却是相同的，都是数组 softirq_vec中定义的action函数*/

/*这里的softirq_vec数组大小为10*/
static struct softirq_action softirq_vec[NR_SOFTIRQS];


```

NR_SOFTIRQS 是枚举类型，定义在文件 include/linux/interrupt.h 中

```c
enum
{
HI_SOFTIRQ=0, /* 高优先级软中断 */
TIMER_SOFTIRQ, /* 定时器软中断 */
NET_TX_SOFTIRQ, /* 网络数据发送软中断 */
NET_RX_SOFTIRQ, /* 网络数据接收软中断 */
BLOCK_SOFTIRQ,
BLOCK_IOPOLL_SOFTIRQ,
TASKLET_SOFTIRQ, /* tasklet 软中断 */
SCHED_SOFTIRQ, /* 调度软中断 */
HRTIMER_SOFTIRQ, /* 高精度定时器软中断 */
RCU_SOFTIRQ, /* RCU 软中断 */
NR_SOFTIRQS
}
```

**如果我们想使用软中断来处理下半部，我们就需要再内核中进行处理，==因此不建议使用==**





#### （2）tasklet

1. 定义和初始化
	```c
	#include <linux/interrupt.h>

	// 定义tasklet结构体
	static struct tasklet_struct my_tasklet;

	// 下半部处理函数
	static void my_tasklet_func(unsigned long data) {
		// 处理延迟任务（如数据解析、状态上报）
	}

	// 初始化tasklet（通常在模块初始化中）
	tasklet_init(&my_tasklet, my_tasklet_func, (unsigned long)dev);
	```

2. 上半部触发下半部，`tasklet_schedule`函数：
   ```c
   static irqreturn_t my_irq_handler(int irq, void *dev_id) {
		// 上半部：快速处理（如清除中断标志）
		tasklet_schedule(&my_tasklet);  // 触发下半部
		return IRQ_HANDLED;
	}

	```




上半部在合适的地方（我的理解：正常情况下，清除中断标志位之后调用tasklet_schedule触发下半部；若中断处理函数一定要在中断里完成，那就在清除中断标志位之前的合适位置调用tasklet_schedule）通过调用tasklet_schedule来执行下半部。




#### （3）工作队列

工作队列是另外一种下半部执行方式，工作队列在**进程上下文**执行，工作队列将要推后的工作交给一个内核线程去执行，因为工作队列工作在进程上下文，因此工作队列允许睡眠或重新调度。因此如果你要推后的工作可以睡眠那么就可以选择工作队列，否则的话就只能选择软中断或 tasklet。


在使用方法上与tasklet非常相似，先定义，初始化，然后在上半部处理函数中调用。

1. 定义和初始化
   ```c
	#include <linux/workqueue.h>

	// 定义工作项
	static struct work_struct my_work;

	// 工作处理函数（进程上下文）
	static void my_work_func(struct work_struct *work) {
		// 可执行阻塞操作（如I/O读写、获取信号量）
		msleep(100);  // 允许睡眠
	}

	// 初始化工作项（通常在模块初始化中）
	INIT_WORK(&my_work, my_work_func);
   ```
2. 上半部触发工作队列
   ```c
   static irqreturn_t my_irq_handler(int irq, void *dev_id) {
		// 上半部：快速处理
		schedule_work(&my_work);  // 提交工作项到共享队列
		return IRQ_HANDLED;
	}
   ```
3. 



#### 中断线程化

中断具有最高的优先级，当有中断产生时，CPU会暂停当前的执行流程，转而去执行中断处理程序。硬件中断处理过程中会关掉中断，如果此时有其它中断产生，那么这些中断将无法及时得到处理，这也是导致内核延迟的一个重要原因。另外，中断优先级比进程高，一旦有中断产生，无论是普通进程还是实时进程都要给中断让路，如果中断处理耗时过长，则会严重影响到系统的实时性。因此内核设计的目标是将中断状态下需要执行的工作量尽量压缩到最低限度。



通过上一个小节的描述，相信大家都确信中断对linux 实时性的最大的敌人。那么怎么破？我曾经接触过一款RTOS，它的中断handler非常简单，就是发送一个inter-task message到该driver thread，对任何的一个驱动都是如此处理。这样，每个中断上下文都变得非常简短，而且每个中断都是一致的。在这样的设计中，外设中断的处理线程化了，然后，系统设计师要仔细的为每个系统中的task分配优先级，确保整个系统的实时性。

在Linux kernel中，一个外设的中断处理被分成top half和bottom half，top half进行最关键，最基本的处理，而比较耗时的操作被放到bottom half（softirq、tasklet）中延迟执行。虽然bottom half被延迟执行，但始终都是先于进程执行的。为何不让这些耗时的bottom half和普通进程公平竞争呢？因此，linux kernel借鉴了RTOS的某些特性，对那些耗时的驱动interrupt handler进行线程化处理，在内核的抢占点上，让线程（无论是内核线程还是用户空间创建的线程，还是驱动的interrupt thread）在一个舞台上竞争CPU。







## 设备树中断信息节点

linux通过读取设备数中的中断属性信息来配置中断。对于中断控制器而言 ，设 备树绑定信息参考文档Documentation/devicetree/bindings/arm/gic.txt。



### 设备树中断信息

在imx6ull.dtsi中，根节点下的intc代表着中断控制器

```c#
/{
......

intc: interrupt-controller@00a01000 {
		compatible = "arm,cortex-a7-gic";	//gic.txt规定了该属性的值
		#interrupt-cells = <3>;				//规定了子节点的interrupt属性要有3个值
		interrupt-controller;				//表明这个节点是中断控制节点
    	/*reg属性指定基本物理地址和GIC寄存器的大小。
    	第一个区域是GIC分发器的寄存器基地址和大小
    	第二个区域是cpu接口的基地址和大小*/
		reg = <0x00a01000 0x1000>,			
		      <0x00a02000 0x100>;
	};
	
......
    
soc{
    ......
    aips1: aips-bus@02000000{		//aips1总线
        ......
        //对于imx6ull，除了CPU专用的中断之外，所有中断都连接到GPC
        gpc: gpc@020dc000 {			
				compatible = "fsl,imx6ul-gpc", "fsl,imx6q-gpc";
				reg = <0x020dc000 0x4000>;		//GPC的基地址和大小
				interrupt-controller;			//表明这个节点是中断控制节点
				#interrupt-cells = <3>;	
            	//第一个参数是共享中断，第二个参数是GPC的中断号，第三个参数是触发方式
				interrupts = <GIC_SPI 89 IRQ_TYPE_LEVEL_HIGH>;
				interrupt-parent = <&intc>;	//指定父节点
				fsl,mf-mix-wakeup-irq = <0xfc00000 0x7d00 0x0 0x1400640>;
			};
        ......
        gpio5: gpio@020ac000 {
				compatible = "fsl,imx6ul-gpio", "fsl,imx35-gpio";
				reg = <0x020ac000 0x4000>;
				interrupts = <GIC_SPI 74 IRQ_TYPE_LEVEL_HIGH>,
					     <GIC_SPI 75 IRQ_TYPE_LEVEL_HIGH>;
				gpio-controller;
				#gpio-cells = <2>;
				interrupt-controller;
				#interrupt-cells = <2>;		//修改了子节点的interrupt属性需要2个值
			};
    }
    ......
}
}
```



在NXP官方开发板的设备树中有一个使用例子，我们要做的就是仿照下面这个例子在设备树中添加中断节点

```c#
//在Documentation/devicetree/bindings/gpio/gpio-mxs.txt有描述
/*
- interrupts : Should be the port interrupt shared by all 32 pins.
- #interrupt-cells : Should be 2.  The first cell is the GPIO number.
  The second cell bits[3:0] is used to specify trigger type and level flags:
      1 = low-to-high edge triggered.
      2 = high-to-low edge triggered.
      4 = active high level-sensitive.
      8 = active low level-sensitive.
*/
&i2c{
	......
		
    fxls8471@1e {
		compatible = "fsl,fxls8471";
		reg = <0x1e>;
		position = <0>;
		interrupt-parent = <&gpio5>;		//指定父中断
		interrupts = <0 8>;					//GPIO5_IO00,低电平触发
	};
	
}
```





### 从设备数获取中断号



通用函数irq_of_parse_and_map可以从interrupt属性获取中断号

```c
unsigned int irq_of_parse_and_map(struct device_node *dev,
									int index)

--dev：设备节点

--index：索引号，interrupt属性可能有多个中断，通过指定index来获取指定的中断号
```



若使用的是gpio中断可以使用下面的函数

```c
int gpio_to_irq(unsigned int gpio)

--gpio：gpio编号
    
--返回值就是中断号
```





## linux内核宏container_of

**==通过结构体成员变量地址推出结构体地址==**

```c
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE*)0)->MEMBER)
#define container_of(ptr, type, member) ({          \
        const typeof(((type *)0)->member)*__mptr = (ptr);    \
    (type *)((char *)__mptr - offsetof(type, member)); })
```

三个参数， ptr是成员变量的指针， type是指结构体的类型， member是成员变量的名字。
