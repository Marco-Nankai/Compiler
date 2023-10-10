.arch armv7-a
.comm i,4
.comm n,4
.comm f,4
.text
.align 2
.section .rodata
.align 2
str0:
       .ascii "%d\000"
       .align  2
str1:
       .ascii "the number is %d\n\000"
       .align  2
       .text
       .global main
main:
       push   {fp,lr}
       add fp,sp,#4
       ldr r0,_bridge+12
       ldr r1,_bridge+4
       bl scanf
       movs r0,#1
       movs r1,#2
       ldr  r2,_bridge+8@f
       ldr  r3,_bridge@i
       str  r0,[r2]
       str  r1,[r3]
.L1:
       ldr r0,_bridge
       ldr r0,[r3]
       ldr r1,_bridge+4
       ldr r1,[r1]
       cmp r0,r1
       ble .L2
       b .L3
.L2:
       ldr r0,_bridge+8
       ldr r0,[r0]
       ldr r1,_bridge
       ldr r1,[r1]
       mul r0,r0,r1
       adds r1,r1,#1
       ldr r3,_bridge+8
       str  r0,[r3]
       ldr r3,_bridge
       str  r1,[r3]
       b .L1 
.L3: 
       ldr r0,_bridge+16
       ldr r1,_bridge+8
       ldr r1,[r1]
       bl printf
       pop {fp,pc}
_bridge:
       .word i @0
       .word n @4
       .word f @8
       .word str0 @12
       .word str1 @16
       .section .note.GNU-stack,"",%progbits
