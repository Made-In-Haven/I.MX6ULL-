# Linux多点电容触摸屏实验

## 一、多点电容触摸屏幕
ATK - 7016 屏幕由 TFT LCD 面板与触摸面板封装而成，底层是 LCD 面板，上层为触摸面板。电容触摸屏需驱动 IC ，ATK - 7016 的触摸 IC 是 FT5426 ，它采用 15*28 驱动结构（15 个感应通道、28 个驱动通道 ），最多支持 5 点电容触摸，通过 I2C 接口（SCL、SDA 为 I2C 引脚，搭配 RST 复位引脚、INT 中断引脚 ）与主控制器连接，借 INT 引脚通知主控制器触摸事件，再于中断服务函数读触摸数据 。和其他 I2C 器件一样，FT5426 靠读写寄存器完成初始化与触摸坐标读取，其 I2C 设备地址为 0X38 ，本章基于已讲的 I.MX6U 的 I2C 知识，对其部分寄存器进行读写操作 。

| 寄存器地址 | 位     | 寄存器功能                     | 描述                                                         |
| ---------- | ------ | ------------------------------ | ------------------------------------------------------------ |
| 0X00       | [6:4]  | 模式寄存器                     | 设置 FT5426 的工作模式：<br/>000：正常模式。<br/>001：系统信息模式<br/>100：测试模式。 |
| 0X02       | [3:0]  | 触摸状态寄存器                 | 记录有多少个触摸点，<br/>有效值为 1~5。                      |
| 0X03       | [7:6]  | 第一个触摸点 X 坐标高位数据    | 事件标志：<br/>00：按下。<br/>01：抬起<br/>10：接触<br/>11：保留 |
|            | [3:0]  |                                | X 轴坐标值高 4 位。                                          |
| 0X04       | [7:0]  | 第一个触摸点 X 坐标低位数据    | X 轴坐标值低 8 位                                            |
| 0X05       | [7:4]<br/>[3:0] | 第一个触摸点 Y 坐标高位数据    | 触摸点的 ID。<br/>Y 轴坐标高 4 位                            |
| 0X06       | [7:0]  | 第一个触摸点 Y 坐标低位数据    | Y 轴坐标低 8 位                                              |
| 0X09       | [7:6]<br/>[3:0] | 第二个触摸点 X 坐标高位数据    | 与寄存器 0X03 含义相同。                                      |
| 0X0A       | [7:0]  | 第二个触摸点 X 坐标低位数据    | 与寄存器 0X04 含义相同。                                      |
| 0X0B       | [7:4]<br/>[3:0] | 第二个触摸点 Y 坐标高位数据    | 与寄存器 0X05 含义相同。                                      |
| 0X0C       | [7:0]  | 第二个触摸点 Y 坐标低位数据    | 与寄存器 0X06 含义相同。                                      |
| 0X0F       | [7:6]<br/>[3:0] | 第三个触摸点 X 坐标高位数据    | 与寄存器 0X03 含义相同。                                      |
| 0X10       | [7:0]  | 第三个触摸点 X 坐标低位数据    | 与寄存器 0X04 含义相同。                                      |
| 0X11       | [7:4]<br/>[3:0] | 第三个触摸点 Y 坐标高位数据    | 与寄存器 0X05 含义相同。                                      |
| 0X12       | [7:0]  | 第三个触摸点 Y 坐标低位数据    | 与寄存器 0X06 含义相同。                                      |
| 0X15       | [7:6]<br/>[3:0] | 第四个触摸点 X 坐标高位数据    | 与寄存器 0X03 含义相同。                                      |
| 0X16       | [7:0]  | 第四个触摸点 X 坐标低位数据    | 与寄存器 0X04 含义相同。                                      |
| 0X17       | [7:4]<br/>[3:0] | 第四个触摸点 Y 坐标高位数据    | 与寄存器 0X05 含义相同。                                      |
| 0X18       | [7:0]  | 第四个触摸点 Y 坐标低位数据    | 与寄存器 0X06 含义相同。                                      |
| 0X1B       | [7:6]<br/>[3:0] | 第五个触摸点 X 坐标高位数据    | 与寄存器 0X03 含义相同。                                      |
| 0X1C       | [7:0]  | 第五个触摸点 X 坐标低位数据    | 与寄存器 0X04 含义相同。                                      |
| 0X1D       | [7:4]<br/>[3:0] | 第五个触摸点 Y 坐标高位数据    | 与寄存器 0X05 含义相同。                                      |
| 0X1E       | [7:0]  | 第五个触摸点 Y 坐标低位数据    | 与寄存器 0X06 含义相同。                                      |
| 0XA1       | [7:0]  | 版本寄存器                     | 版本高字节                                                   |
| 0XA2       | [7:0]  | 版本寄存器                     | 版本低字节                                                   |
| 0XA4       | [7:0]  | 中断模式寄存器                 | 用于设置中断模式：<br/>0：轮询模式<br/>1：触发模式           |


## 二、多点触摸（MT）协议

**电容触摸屏驱动其实是一下几种Linux驱动的组合：**
- **IIC驱动：电容触摸IC基本都是IIC接口**
- **中断：电容触摸IC通过中断上报触摸信息，所以Linux这边也需要用中断来处理**
- **Input子系统：向用户空间上报触摸点的坐标、按下动作、抬起动作**

Linux内核关于多点触摸协议的文档在`Documentation/input/multi-touch-protocol.txt`。MT协议被分为2种类型：

- Type A：适用于触摸点不能被区分或追踪（在实际使用中非常少见）
- Type B：使用于有硬件跟踪并能区分触摸点的设备，此类型设备通过`slot`更新某个触摸点的信息。一般的多点触摸屏IC都有这个能力。

Type B设备需要给每个识别出的触摸点分配一个slot，然后使用这个slot来上报触摸点信息

### 2.1、上报事件类型
触摸点的信息通过一系列的`ABS_MT`事件上报给Linux内核，`ABS_MT`事件定义在`include/uapi/linux/input.h`:

```c
#define ABS_MT_SLOT		    0x2f	/* 正在修改的MT触摸点槽位 */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TOUCH_MINOR	0x31	/* Minor axis (omit if circular) */
#define ABS_MT_WIDTH_MAJOR	0x32	/* Major axis of approaching ellipse */
#define ABS_MT_WIDTH_MINOR	0x33	/* Minor axis (omit if circular) */
#define ABS_MT_ORIENTATION	0x34	/* Ellipse orientation */
#define ABS_MT_POSITION_X	0x35	/* 触摸点中心X坐标 */
#define ABS_MT_POSITION_Y	0x36	/* 触摸点中心Y坐标 */
#define ABS_MT_TOOL_TYPE	0x37	/* 触摸设备类型 */
#define ABS_MT_BLOB_ID		0x38	/* 将一组数据包分组为Blob */
#define ABS_MT_TRACKING_ID	0x39	/* 触摸点的唯一跟踪ID */
#define ABS_MT_PRESSURE		0x3a	/* 触摸区域压力值 */
#define ABS_MT_DISTANCE		0x3b	/* 接触悬停距离 */
#define ABS_MT_TOOL_X		0x3c	/* Center X tool position */
#define ABS_MT_TOOL_Y		0x3d	/* Center Y tool position */
```


### 2.2、Type B触摸点信息上报时序

对于Type B类型的设备，发送触摸点信息的时序如下：
```c
ABS_MT_SLOT 0           //上报触摸点对应的slot，这个slot就是触摸点的ID，由触摸屏的IC提供
ABS_MT_TRACKING_ID 45   //
ABS_MT_POSITION_X x[0]
ABS_MT_POSITION_Y y[0]

ABS_MT_SLOT 1
ABS_MT_TRACKING_ID 46
ABS_MT_POSITION_X x[1]
ABS_MT_POSITION_Y y[1]

SYN_REPORT
```

对于slot 0:
1. 上报触摸点对应的slot，这个slot就是触摸点的ID，由触摸屏的IC提供.
    ```c
    void input_mt_slot(struct input_dev *dev, int slot)
    ```
2. 每个slot都要关联一个TRACKING_ID，通过修改slot关联的TRACKING_ID来完成对触摸点的添加替换或删除。
   ```c
   #define MT_TOOL_FINGER		0x00        //意思是用手指触摸屏幕
   #define MT_TOOL_PEN		0x01
   #define MT_TOOL_PALM		0x02
   #define MT_TOOL_DIAL		0x0a
   #define MT_TOOL_MAX		0x0f
   void input_mt_report_slot_state(struct input_dev *dev, unsigned int tool_type, bool active);

   --active: 如果是新添加一个触摸点，active的值要设为true。
             如果是删除一个触摸点，active的值要设为false

   ```

3. 上报触摸点的x坐标
   ```c
   void input_report_abs(struct input_dev *dev, unsigned int code, int value)
   ```
4. 上报触摸点的y坐标
   ```c
   void input_report_abs(struct input_dev *dev, unsigned int code, int value)
   ```
5. 使用`input_sync`函数上报`SYN_REPORT`事件


上报例子：
```c
struct finger {
	u8 x_low;
	u8 x_high;
	u8 y_low;
	u8 y_high;
} __packed;

struct touchdata {
	u8 status;
	struct finger finger[MAX_TOUCHES];
} __packed;

static void ili210x_report_events(struct input_dev *input,
				  const struct touchdata *touchdata)
{
	int i;
	bool touch;
	unsigned int x, y;
	const struct finger *finger;

	for (i = 0; i < MAX_TOUCHES; i++) {
		input_mt_slot(input, i);            //上报slot

		finger = &touchdata->finger[i];

		touch = touchdata->status & (1 << i);
		input_mt_report_slot_state(input, MT_TOOL_FINGER, touch);       //给slot关联一个tracking_id，指定触摸类型
		if (touch) {
			x = finger->x_low | (finger->x_high << 8);
			y = finger->y_low | (finger->y_high << 8);

			input_report_abs(input, ABS_MT_POSITION_X, x);              //上报触摸点坐标
			input_report_abs(input, ABS_MT_POSITION_Y, y);
		}
	}

	input_mt_report_pointer_emulation(input, false);
	input_sync(input);
}
```


## 三、多点触摸用到的API函数

### 3.1、初始化slot函数
该函数定义在`drivers/input/input-mt.c`，写驱动必须先调用这个初始化函数
```c

/*
 * input_mt_init_slots() - 初始化MT的输入slots
 * @dev
 * @nums_slots : 设备要使用的slot数量，也就是触摸点的数量
 * @flags  mt tasks to handle in core
 */
#define INPUT_MT_POINTER	0x0001	/* pointer device, e.g. trackpad */
#define INPUT_MT_DIRECT		0x0002	/* direct device, e.g. touchscreen */
#define INPUT_MT_DROP_UNUSED	0x0004	/* drop contacts not seen in frame */
#define INPUT_MT_TRACK		0x0008	/* use in-kernel tracking */
#define INPUT_MT_SEMI_MT	0x0010	/* semi-mt device, finger count handled manually */

int input_mt_init_slots(struct input_dev *dev, unsigned int num_slots,
                        unsigned int flags)


```

### 3.2、上报函数

```c
/* input_mt_slot() - 告诉内核上报的是哪个触摸点的坐标
 */
void input_mt_slot(struct input_dev *dev, int slot)
```

```c
#define MT_TOOL_FINGER		0x00        //意思是用手指触摸屏幕
#define MT_TOOL_PEN		0x01
#define MT_TOOL_PALM		0x02
#define MT_TOOL_DIAL		0x0a
#define MT_TOOL_MAX		0x0f

/* input_mt_report_slot_state() - 给slot关联一个tracking_id，指定触摸类型
 */
void input_mt_report_slot_state(struct input_dev *dev, unsigned int tool_type, bool active);

--active: 如果是新添加一个触摸点，active的值要设为true。
            如果是删除一个触摸点，active的值要设为false。

```

```c
/* input_report_abs() - 上传坐标
 */
void input_report_abs(struct input_dev *dev, unsigned int code, int value);
```

```c
/**
 * 如果追踪到的触摸点数量多于当前上报的数量，驱动程序使用 BTN_TOOL_TAP 事件来通知用户空间当前追踪到的触摸点总数量，然后调用 input_mt_report_pointer_emulation 函数将use_count 参数设置为 false。
 * 否则的话将 use_count 参数设置为 true，表示当前的触摸点数量(此函数会根据上报的ABS_MT_TRACKING_ID获取到具体的触摸点数量)，此函数定义在文件 drivers/input/input-mt.c中，函数原型如下：
 */


void input_mt_report_pointer_emulation(struct input_dev *dev, bool use_count);
```


## 四、中断线程化
硬件中断具有最高优先级，不管内核在执行什么操作，一旦硬件中断到来，内核必须立刻去处理硬件中断。如果中断频繁发生，那么内核就频繁地去处理中断，导致其他重要的任务得不到即使的处理。

中断线程化会使中断作为内核线程运行，而且可以被赋予不同的优先级。在这种情况下，只要把重要的任务优先级设高一点就可以保证高优先级的任务被及时处理。对于不太重要的中断就可以使用中断线程化的处理方式

### 4.1、devm_request_threaded_irq

```c
/**
 * request_threaded_irq() - 申请线程中断化
 * @irq: 中断号
 * @handler: 发生中断时首先要执行的硬中断处理函数，这个函数可以通过返回 IRQ_WAKE_THREAD唤醒中断线程，也可返回IRQ_HANDLE不* 执行中断线程.若该变量设为NULL，则一定要在irqflags设置IRQF_ONESHOT
 * @thread_fn: 中断线程，类似于中断下半部
 * @irqflags: 标志，定义在内核中的 include/linux/interrupt.h。下面列举一些常用的中断标志：
		IRQF_SHARED:多个设备共享一个中断线，共享的所有中断都必须指定此标志。如果使用共享中断的话，request_irq函数的 dev 参数就是唯一区分他们的标志
		IRQF_ONESHOT：单次中断，执行一次就结束。中断会在中断线程执行完成后才会再次打开。
		IRQF_TRIGGER_RISING：上升沿触发
		IRQF_TRIGGER_FALLING：下降沿触发
		IRQF_TRIGGER_HIGH：高电平触发
		IRQF_TRIGGER_LOW：低电平触发
 * @name: 自定义的中断名字
 * @dev: 传递给 irq_handler_t 的第二个参数。
 */

int request_threaded_irq(unsigned int irq, irq_handler_t handler,
		     irq_handler_t thread_fn,
		     unsigned long flags, const char *name, void *dev);


int devm_request_threaded_irq(struct device *dev, unsigned int irq,
			  irq_handler_t handler, irq_handler_t thread_fn,
			  unsigned long irqflags, const char *devname,
			  void *dev_id);
```

`devm_request_threaded_irq`和`request_threaded_irq`效果一样，都是申请中断。不同的是`devm_request_threaded_irq`会在卸载驱动时自动释放申请到的中断。

==**所有使用`devm_`前缀的函数申请到的资源都会在卸载驱动时自动释放，不用手动释放。**==








