# RTC（Real-Time Clock）实验
RTC（实时时钟）是一种用于保持时间和日期的电子设备。即使在系统掉电或重启时，RTC也能保持准确的时间信息，一般依赖电池供电（如CR2032纽扣电池）。



## RTC



IMX6ULL的RTC内容在SNVS章节。6U的RTC分为LP和HP。LP叫做SRTC，HP叫做RTC

SNVS_LP: dedicated always-powered-on domain，板子掉电后需要用其他电源供电，例如纽扣电池

SNVS_HP: system (chip) power domain，和板子一起供电

IMX6ULL的RTC不是很准

RTC很类似定时器，外接32.768khz时钟，计数寄存器分为2个

RTC使用很简单，只需要打开RTC，然后读取RTC计数器的值，获取时间值；或者向RTC计数器写入值，调整时间



## RTC寄存器

SNVS_HPCOMR寄存器: bit31置1

SNVS_LPCR寄存器: bit0置1，开启SRTC功能

SNVS_LPSRTCMR寄存器：bit0-14为计数器的一部分 ，This register can be programmed only when SRTC is not active
and not locked

SNVS_LPSRTCLR寄存器：bit0-31也是计数器的一部分，**时钟是32.768khz，时钟每一次上升沿，该寄存器加一，一秒是1000 0000 0000 0000（二进制），低15位每溢出一位，表示过去一秒。所以，SNVS_LPSRTCLR寄存器的bit16-31才代表真正的秒数**

IMX6ULL点RTC模式默认从1970年1月1日0时0分0秒





## 问题

32位系统下，long占4个字节，long long占8个字节

64位系统，2种都占8字节









