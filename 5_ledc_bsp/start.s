# 0 "project/start.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "project/start.S"
# 13 "project/start.S"
.global _start





_start:


 mrs r0, cpsr
 bic r0, r0, #0x1f
 orr r0, r0, #0x13
 msr cpsr, r0







 ldr sp,=0X80200000
 b main
