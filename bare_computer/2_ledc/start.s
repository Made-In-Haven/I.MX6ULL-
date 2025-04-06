.global _start

_start:
    /*  设置处理器模式，设置为SVC */
    mrs r0,cpsr  /*读取cpsr */
    bic r0,r0, #0x1f
    orr r0,r0, #0x13
    msr cpsr,r0 /*将r0写入cpsr */

    ldr sp, =0x80200000
    b main
