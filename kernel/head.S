#include "linkage.h"

.code64
.section .text
.global _start


_start:
    mov $0x10,%ax
    mov %ax,%ds
    mov %ax,%es
    mov %ax,%fs
    mov %ax,%ss
    mov $0x7e00,%esp

    #加载GDT(通过偏移量访问GDT指针)
    lgdt GDT_POINTER(%rip)

    #加载IDT
    lidt IDT_POINTER(%rip)

    mov $0x10,%ax
    mov %ax,%ds
    mov %ax,%es
    mov %ax,%fs
    mov %ax,%gs
    mov %ax,%ss
    movq $0x7e00,%rsp

    #加载(换到新地址去)
    movq $0x101000,%rax
    movq %rax,%cr3
    
    movq switch_seg(%rip),%rax
    pushq $0x08   /*选择子*/
    pushq %rax    /*跳板*/
    lretq         /*相当于ret通过栈返回*/

    #64位入口
switch_seg:
    .quad entry64
    
entry64:
    movq $0x10,%rax
    movq %rax,%ds
    movq %rax,%es
    movq %rax,%gs
    movq %rax,%ss
    movq _stack_start(%rip),%rsp


setup_IDT:
    /*
    %rdx:高8字节
    %rax:低8字节
    */
    leaq ignore_init(%rip),%rdx  #在设置中断之前使用默认中断处理函数(后期修改)
    movq $(0x08 << 16),%rax   #代码段选择子
    movw %dx,%ax              #被调用函数在代码段内的偏移量
    movq $(0x8E00 << 32),%rcx
    addq %rcx,%rax
    movl %edx,%ecx
    shrl $16,%ecx
    shlq $48,%rcx
    addq %rcx,%rax
    shrq $32,%rdx   #在高8字节中段内偏移是高32位,所以需要右移
    leaq IDT_Table(%rip),%rdi  #把描述符放置到IDT的位置
    mov $256,%rcx           #设置循环次数


rp_sidt:
    movq %rax,(%rdi)
    movq %rdx,8(%rdi)
    addq $0x10,%rdi   #放置到下一个16字节的区域
    dec %rcx
    jne rp_sidt


setup_TSS64:
    leaq TSS64_Table(%rip),%rdx
    xorq %rax,%rax
    xorq %rcx,%rcx
    movq $0x89,%rax
    shlq $40,%rax
    movl %edx,%ecx
    shrl $24,%ecx
    shlq $56,%rcx
    addq %rcx,%rax
    xorq %rcx,%rcx
    movl %edx,%ecx
    andl $0xffffff,%ecx
    shlq $16,%rcx
    addq %rcx,%rax
    addq $103,%rax
    leaq GDT_Table(%rip),%rdi
    movq %rax,80(%rdi)
    shrq $32,%rdx
    movq %rdx,88(%rdi)
    
    #mov $0x40,%ax
    #ltr %ax



    movq go_to_kernel(%rip),%rax
    pushq $0x08  #代码段选择子
    pushq %rax   #kernel地址
    lretq        #跳入c内核

go_to_kernel:
    .quad   Start_Kernel



/* 产生中断时进入这里 */
ignore_init:
    cld
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rdi
    pushq %rsi

    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %es,%rax
    pushq %rax
    movq %ds,%rax
    pushq %rax

    movq $0x10,%rax
    movq %rax,%ds
    movq %rax,%es

    leaq int_msg(%rip),%rax
    pushq %rax

    movq %rax,%rdx
    movq $0x00000000,%rsi
    movq $0x00ff0000,%rdi
    movq $0,%rax
    callq color_printk
    addq $0x08,%rsp  #跳过color_printk函数的地址

Loop:
    jmp Loop

    popq %rax
    movq %rax,%ds
    popq %rax
    movq %rax,%es

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8

    popq %rsi
    popq %rdi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    iretq
    


int_msg:
    .asciz "Unknow interrupt or fault at RIP\n"

ENTRY(_stack_start)
    .quad init_task_union + 32768 #内核栈


.align 8

.org 0x1000  #.quad为64位
__PML4E:
    .quad   0x102007
    .fill   255,8,0

    .quad   0x102007
    .fill   255,8,0



.org 0x2000
__PDPTE:
    .quad   0x103007 #采用非1GB的小页(用户模式)(可读写)
    .fill   511,8,0



.org 0x3000
__PDE:
    /* 10MB */
    .quad   0x000087 #1000 0111(采用2MB大页)(用户模式)(可读写)
    .quad   0x200087
    .quad   0x400087
    .quad   0x600087
    .quad   0x800087

    /* 16MB */
    .quad   0xe0000087 #PAT打开
    .quad   0xe0200087
    .quad   0xe0400087
    .quad   0xe0600087
    .quad   0xe0800087
    .quad   0xe0a00087
    .quad   0xe0c00087
    .quad   0xe0e00087
    .fill   499,8,0



.section .data

.global GDT_Table
GDT_Table:
    .quad   0x0000000000000000  /* 空描述符 */
    .quad   0x0020980000000000  /* 内核64位代码段 */
    .quad   0x0000920000000000  /* 内核64位数据段 */
    .quad   0x0000000000000000  /* 用户32位代码段 */
    .quad   0x0000000000000000  /* 用户32位数据段 */
    .quad   0x0020f80000000000  /* 用户64位代码段(IA32_SYSENTER_CS) */
    .quad   0x0000f20000000000  /* 用户64位数据段(IA32_SYSENTER_SS) */
    .quad   0x00cf9a000000ffff  /* 内核32位代码段 */
    .quad   0x00cf92000000ffff  /* 内核32位数据段 */
    .fill   10,8,0
GDT_END:

GDT_POINTER:
GDT_LIMIT:  .word   GDT_END - GDT_Table - 1
GDT_BASE:   .quad   GDT_Table


#IDT
.global IDT_Table
IDT_Table:
    .fill 512,8,0   #IDT最多支持256个(8字节对齐)
IDT_END:

IDT_POINTER:
IDT_LIMIT:  .word   IDT_END - IDT_Table - 1
IDT_BASE:   .quad   IDT_Table



#TSS64
.global TSS64_Table
TSS64_Table:
    .fill 13,8,0   #预留13个TSS槽位(每个TSS8字节)
TSS64_END:

TSS64_LIMIT:    .word   TSS64_END - TSS64_Table - 1
TSS64_BASE:     .quad   TSS64_Table

