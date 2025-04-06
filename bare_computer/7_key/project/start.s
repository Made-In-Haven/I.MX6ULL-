.global _start
.global _bss_start
_bss_start:
    .word __bss_start
.global _bss_end
_bss_end:
    .word __bss_end

_start:
    mrs r0,cpsr
    bic r0,r0,#0x1f
    orr r0,r0, #0x13
    msr cpsr,r0 /*将r0写入cpsr，切换到svc模式*/

    //清除bss段
    ldr r0,_bss_start
    ldr r1,_bss_end
    mov r2,#0
bss_loop:
    stmia r0!,{r2} //这个指令表示将r2寄存器中的数据依次存储到内存地址r0开始的位置，每存储一个寄存器，r0的地址增加4。
    cmp r0,r1  //比较r0和r1的值
    ble bss_loop //如果r0中的地址小于r1中的地址，继续清除bss段

    //设置sp指针，配置c语言
    ldr sp, =0x80200000
    b main
    