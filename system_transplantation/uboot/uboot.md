# U-Boot

Bootrom（或Boot ROM）是嵌入处理器芯片内的一小块掩模ROM或写保护闪存。它包含处理器在上电或复位时执行的第一个代码。

**在计算机启动过程中，首先执行BootROM中的启动程序，然后由BootROM加载BootLoader，最后由BootLoader加载操作系统内核。**

uboot就是一个bootloader，作用就是启动linux或者其他系统。==**这段 bootloader 程序会先初始化 DDR 等外设(I.MX6ULL的bootrom已经包含了ddr的初始化，因此之后实验的uboot不会包含初始化ddr的内容，！！！但是不是所有芯片的bootrom都会初始化ddr)**==，然后将Linux内核从 flash（NAND、NOR FLASH、SD、MMC 等）（一般**linux镜像(zImage or uImage文件)**和**设备树(.dtb文件)**存放在外部flash中）拷贝到 DDR 中，最后启动 Linux 内核。==（DDR掉电数据消失，所以不能把镜像和设备树直接放在DDR中，就像不能在windows把文件直接放在内存，得放在磁盘）==



bootloader 的实际工作要复杂的多，但是它最主要的工作就是启动 Linux 内核， bootloader 和 Linux 内核的关系就像是 PC 上的 BIOS 和 Windows 的关系一样， bootloader 就相当于 BIOS。

目前有很多现成的 bootloader 软件可以使用，比如 U-Boot、 vivi、 RedBoot 等等，其中以 U-Boot 使用最为广泛，为了方便书写，后边的笔记会将 U-Boot 写为 uboot，毕竟是笔记，能理解就可以啦。

uboot 的全称是 Universal Boot Loader， 它是一个遵循 GPL 协议的开源软件， 是一个裸机代码，我们可以将它看作是一个裸机综合例程。现在的 uboot 已经支持液晶屏、网络、 USB 等高级功能。



## uboot编译



## uboot命令

### help命令

- help命令：输入help后会打印出当前 uboot 所支持的所有命令

  查看某一个命令的信息 输入?+命令或者help+命令，例如: `?mmc` 或 `help mmc`
  
  
### go命令

go命令用于执行指定地址的代码
```
go addr [arg ...]
```
```
nfs 87800000 $serverip:/nfs/uart.bin
go 87800000
```

上面的代码就是用nfs把代码下载到87800000，然后用uboot运行裸机代码




### 信息查询 

- bdinfo命令：bdinfo 命令，此命令用于查看板子信息

- printenv命令：该命令主要用于打印出所有的环境变量信息



### 环境变量设置

- **==setenv==**：设置环境变量

  ```Usage:
  setenv [-f] name value ...
      - [forcibly] set environment variable 'name' to 'value ...'
  setenv [-f] name
      - [forcibly] delete environment variable 'name'
  ```
  有时候我们修改的环境变量值可能会有空格， 比如bootcmd、 bootargs 等， 这个时候环境变量值就得用单引号括起来（后便会介绍这两个环境变量）。例如`setenv bootcmd 'console=ttymxc0,115200 root=/dev/mmcblk1p2 rootwait rw'`

- ==**saveenv**==：保存环境变量到mmc

### 内存操作（DDR）

- md命令
  ```
  md[.b, .w, .l] address [# of objects]
  [.b .w .l]：对应byte、word和long ，也就是分别以1个字节 、2个字节、4个字节来显示内存值。
  address：要查看的内存起始地址。
  [# of objects]：表示要查看的数据长度，这个数据长度单位不是字节，而是跟我们所选择的显示格式有关。比如我们设置要查看的内存长度为20(十六进制为 0x14 )，如果显示格式为.b 的话那就表示20个字节；如果显示格式为.w的话就表示20个word，也就是20*2=40个字节；如果显示格式为.l的话就表示20个long，也就是20*4=80个字节。
  ```

​	uboot中显示的数据都是16进制的，因此想要使用md命令查看20个字节的数据需要输入：`md.b 0x80000000 14`（20的16	进制是14）

- nm 命令：用于修改指定地址的内存值，使用 q 退出内存的修改。
- mm 命令：也是修改指定地址内存值的，使用 mm 修改内存值的时候地址会自增，而使用命令 nm 的话地址不会自增。
- mw ：用于使用一个指定的数据填充一段内存
- cp 是数据拷贝命令，用于将 DRAM 中的数据从一段内存拷贝到另一段内存中，或者把 Nor Flash 中的数据拷贝到 DRAM 中。
- cmp 是比较命令，用于比较两段内存的数据是否相等。

### 存储器操作（mmc）

uboot 支持 EMMC 和 SD 卡，因此也会提供 EMMC 和 SD 卡的操作命令。 关于存储器的访问主要是 mmc 命令

- mmc read 命令用于将 MMC 中指定扇区中的内容读取到内存中指定的地址，命令格式如下：
	```
	mmc read <dev_num> addr blk# cnt
	```
	- dev_num ： mmc 的设备号，可以通过 mmc list 查询到，这个参数可以不写，不写的话默认为当前 mmc 设备
	
	- addr ：数据读取到内存中的起始地址
	
	- blk# ：要读取的块起始地址 (十六进制 )，一个块是 512 字节，这里的块和扇区是一个意思，在 MMC 设备中我们通常说扇区。
	
	- cnt ：要读取的块数量 (十六进制 )。
	
	  

### 网络操作

nfs



### 文件系统操作

对于I.MX6ULL来说，SD/EMMC分为3个分区：

1. 存放uboot
2. 存放zImage和dtb
3. 存放根文件系统

fat格式的文件系统命令主要有fatinfo、fatls、fstype、fatload、fatwrite

1. fatinfo

2. fatls

3. fstype

4. ==fatload==

5. fatwrite

uboot 有 ext2 和 ext4 这两种格式的文件系统的操作命令，常用的就四个命令，分别为：ext2load、 ext2ls、 ext4load、 ext4ls 和ext4write。这些命令的含义和使用与 fatload、 fatls 和 fatwrite一样，只是 ext2 和 ext4 都是针对 ext 文件系统的。



### boot操作

uboot的本质是（boot）引导启动Linux，常用的boot命令有：bootz、bootm、boot。

- bootz：要启动 Linux，需要先将 Linux 镜像文件拷贝到 DRAM 中，如果使用到设备树的话也需要将设备树拷贝到 DRAM 中。可以从 EMMC 或者 NAND 等存储设备中将 Linux 镜像和设备树文件拷贝到 DRAM，也可以通过 nfs 或者 tftp 将 Linux 镜像文件和设备树文件下载到 DRAM 中。不管用那种方法，只要能将 Linux 镜像和设备树文件存到 DRAM 中就行，然后使用 bootz 命令来启动， bootz 命令用于启动 zImage **镜像文件**
	```
	bootz [addr [initrd[:size]] [fdt]]
	```
	命令 bootz 有三个参数， addr 是 Linux 镜像文件在 DRAM 中的位置，initrd 是 initrd 文件在DRAM 中的地址，如果不使用 initrd 的话使用‘-’代替即可， fdt 就是设备树文件在 DRAM 中的地址。==实测，只要zImage和dtb文件的地址没有重叠就可以==
  
  
  
- bootm：用于启动uImage

- boot：boot 命令也是用来启动 Linux 系统的，只是 boot 会读取环境变量 bootcmd 来启动 Linux 系统， bootcmd 是一个很重要的环境变量！其名字分为“boot”和“cmd”，也就是“引导”和“命令”，说明这个环境变量保存着引导命令，其实就是启动的命令集合，具体的引导命令内容是可以修改的。例如：

	```
	setenv bootcmd 'tftp 80800000 zImage; tftp 83000000 imx6ull-14x14-emmc-7-1024x600-c.dtb;
	bootz 80800000 - 83000000
	```





