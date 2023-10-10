.arch armv7-a
.comm a,4
.comm b,4
.comm t,4
.comm i,4
.comm n,4
.text
.align 2
.section .rodata
.align 2
str0:
       .ascii "%d\000"
       .align  2
str1:
       .ascii "the first number is %d\n\000"
       .align  2
str2:
       .ascii "the second number is %d\n\000"
       .align  2
str3:
       .ascii "the number is %d\n\000"
       .align  2
       .text
       .global main
main:
       push   {fp,lr}
       add fp,sp,#4
       ldr   r1,_bridge @r1里面给上a的地址
       ldr   r2,_bridge+4 @b的地址
       ldr   r3,_bridge+12   @i的地址     
       movs  r0,#0
       str r0,[r1]
       movs  r0,#1
       str r0,[r3]
       str r0,[r2]
       ldr r0,_bridge+20  @开始准备输入函数的参数
       ldr r1,_bridge+16
       bl scanf
       ldr r0,_bridge+24
       ldr r1,_bridge
       ldr r1,[r1]
       bl printf
       ldr r0,_bridge+28
       ldr r2,_bridge+4
       ldr r2,[r2]
       mov r1,r2
       bl printf
       ldr r0,_bridge+32
       ldr r1,_bridge+16
       ldr r1,[r1]
       bl printf
       b .L1
.L1:   
       ldr r4,_bridge+12
       ldr r4,[r4]
       ldr r3,_bridge+16
       ldr r3,[r3]
       cmp r4,r3
       blt .L2
       b   .L3
.L2:   
       ldr r0,_bridge + 8 @把t的地址给进来
       ldr r2,_bridge+4
       ldr r2,[r2]       
       str r2,[r0] @ t = b
       ldr r1,_bridge
       ldr r1,[r1]
       add r2,r2,r1 @r2中是b的值 
       ldr r4,_bridge+4
       str r2,[r4]
       ldr r0,_bridge + 32
       mov r1,r2
       bl printf
       ldr r6,_bridge+8
       ldr r0,[r6]
       ldr r6,_bridge
       str r0,[r6] @a=t
       ldr r3,_bridge+12
       ldr r3,[r3]
       adds r3,r3,#1
       ldr r6,_bridge+12
       str r3,[r6]
       b .L1
.L3:  
       mov r0, #0
       pop {fp,pc}
_bridge:
       .word a
       .word b
       .word t
       .word i
       .word n @16
       .word str0
       .word str1
       .word str2
       .word str3
       .section .note.GNU-stack,"",%progbits
