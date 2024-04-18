#include "linkage.h"

.code64
.section .text
.global _start #内核程序的开始


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
    movq _stack_start(%rip),%rsp


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
    movq _stack_start(%rip),%rsp  #更新rsp为内核栈的地址(_stack_start是动态的)

    movq $0x1b,%rcx
    rdmsr
    bt $8,%rax
    jnc start_smp


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


setup_TSS64: #设置BSP的TSS描述符,AP不要到这里执行(AVL位保留)
    leaq init_tss(%rip),%rdx #当前TSS描述符指向TSS64_Table
    xorq %rax,%rax
    xorq %rcx,%rcx
    movq $0x89,%rax  #1000 1001 存在 ring0 Type位表明这是TSS描述符
    shlq $40,%rax    #左移到目的位置(eax)
    movl %edx,%ecx   #TSS64_Table地址的低32位
    shrl $24,%ecx    #把低24位去掉,ecx变成TSS64_Table低32地址的高8位(即31:24)
    shlq $56,%rcx    #把TSS64_Table地址的低8位移动到目的位置(ecx)
    addq %rcx,%rax   #合成eax和ecx
    xorq %rcx,%rcx
    movl %edx,%ecx
    andl $0xffffff,%ecx
    shlq $16,%rcx    #TSS64_Table的低24位
    addq %rcx,%rax   #用加法
    addq $103,%rax   #长度固定为104-1(每个核心只使用TSS表的第一个项目)
    leaq GDT_Table(%rip),%rdi   #把制作好的TSS描述符注册到GDT_Table里
    movq %rax,80(%rdi)    #将计算出的TSS的高64位存储到GDT中
    shrq $32,%rdx         #TSS_Table的高32位
    movq %rdx,88(%rdi)    #将计算出的TSS的低64位存储到GDT中
    
    #mov $0x40,%ax
    #ltr %ax



    movq go_to_kernel(%rip),%rax
    pushq $0x08  #代码段选择子
    pushq %rax   #kernel地址
    lretq        #跳入c内核

go_to_kernel:
    .quad   Start_Kernel



start_smp:
    movq go_to_smp_kernel(%rip),%rax
    pushq $0x08
    pushq %rax
    lretq

go_to_smp_kernel:
    .quad Start_SMP





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
    .quad init_task_union + 32768 #内核栈(0x100000~0x120000)


.align 8,0

.org 0x1000  #.quad为64位
__PML4E:
    .quad   0x102003 #0x102000(地址) + 0x003(属性)
    .fill   255,8,0

    .quad   0x102003 #0x101800
    .fill   255,8,0



.org 0x2000
__PDPTE:
    .quad   0x103003 #采用非1GB的小页(可读写)
    .fill   511,8,0



#PDE本身映射到一个更大的物理内存块2MB,而不是指向某一个页表
.org 0x3000
__PDE:
    .quad   0x000083 #1000 0111(采用2MB大页)(可读写)
    .quad   0x200083
    .quad   0x400083
    .quad   0x600083
    .quad   0x800083
    .quad   0xa00083
    .quad   0xc00083
    .quad   0xe00083
    .quad   0x1000083
    .quad   0x1200083
    .quad   0x1400083
    .quad   0x1600083
    .quad   0x1800083
    .quad   0x1a00083
    .quad   0x1c00083
    .quad   0x1e00083
    .quad   0x2000083
    .quad   0x2200083
    .quad   0x2400083
    .quad   0x2600083
    .quad   0x2800083
    .quad   0x2a00083
    .quad   0x2c00083
    .quad   0x2e00083



    /* 16MB */
    .quad   0xe0000083 #显示区(3.5GB)
    .quad   0xe0200083
    .quad   0xe0400083
    .quad   0xe0600083
    .quad   0xe0800083
    .quad   0xe0a00083
    .quad   0xe0c00083
    .quad   0xe0e00083
    .fill   480,8,0



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
    .fill   100,8,0
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
