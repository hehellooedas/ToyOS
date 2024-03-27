#include "linkage.h"


/*
 * AP核心(Application Processor)启动
*/


.balign 0x1000 #4KB边界对齐

.text
.code16

ENTRY(_APU_boot_start)
_APU_boot_base = .

    cli
    wbinvd


    mov %cs,%ax
    mov %ax,%ds
    mov %ax,%es
    mov %ax,%fs
    mov %ax,%ss
    mov %ax,%gs

    # 设置sp栈顶
    movl $(_APU_boot_tmp_Stack_end - _APU_boot_base),%esp

    mov %cs,%ax
    movzx %ax,%esi
    shll $4,%esi

    leal (_APU_Code32 - _APU_boot_base)(%esi),%eax
    movl %eax,_APU_Code32_vector - _APU_boot_base

    leal (_APU_Code64 - _APU_boot_base)(%esi),%eax
    movl %eax,_APU_Code64_vector - _APU_boot_base

    leal (_APU_tmp_GDT - _APU_boot_base)(%esi),%eax
    movl %eax,(_APU_tmp_GDT + 2 - _APU_boot_base)

    #开启保护模式
    lidtl _APU_tmp_IDT - _APU_boot_base
    lgdtl _APU_tmp_GDT - _APU_boot_base

    smsw %ax
    bts $0,%ax
    lmsw %ax


    ljmpl *(_APU_Code32_vector - _APU_boot_base)




.code32
.balign 4
_APU_Code32:

    mov $0x10,%ax
    mov %ax,%ds
    mov %ax,%es
    mov %ax,%fs
    mov %ax,%gs
    mov %ax,%ss

    leal (_APU_boot_tmp_Stack_end - _APU_boot_base)(%esi),%eax
    movl %eax,%esp

    #开启PAE
    movl %cr4,%eax
    bts $5,%eax
    movl %eax,%cr4

    #开启分页模式
    movl $0x90000,%eax
    movl %eax,%cr3

    movl $0xc0000080,%ecx
    rdmsr
    bts $8,%eax
    wrmsr


    movl %cr0,  %eax
    bts  $0,    %eax
    bts  $31,   %eax
    movl %eax,  %cr0

    ljmpl *(_APU_Code64_vector - _APU_boot_base)(%esi)







.code64
.balign 4
_APU_Code64:
    movq $0x20,%rax
    movq %rax,%ds
    movq %rax,%es
    movq %rax,%fs
    movq %rax,%gs
    movq %rax,%ss

    movq $0x100000,%rax
    jmpq *%rax

    hlt



# 数据区

.balign 4
_APU_tmp_IDT:
    .word 0
    .word 0,0

.balign 4
_APU_tmp_GDT:
    .short _APU_tmp_GDT_end - _APU_tmp_GDT - 1
    .long _APU_tmp_GDT - _APU_boot_base
    .short 0
    .quad 0x00cf9a000000ffff
    .quad 0x00cf92000000ffff
    .quad 0x0020980000000000
    .quad 0x0000920000000000
_APU_tmp_GDT_end:




.balign 4
_APU_Code32_vector:
    .long _APU_Code32 - _APU_boot_base
    .word 0x08,0



.balign 4
_APU_Code64_vector:
    .long _APU_Code64 - _APU_boot_base
    .word 0x18,0



_APU_boot_tmp_Stack_start:
    .org 0x400
_APU_boot_tmp_Stack_end:

ENTRY(_APU_boot_end)