#include "linkage.h"


/*
 * AP核心(Application Processor)启动位置
*/


.balign 0x1000 #4KB边界对齐

.text
.code16

ENTRY(_APU_boot_start)
._APU_boot_base = .

    cli
    wbinvd

    mov %cs,%ax
    mov %ax,%dx
    mov %ax,%es
    mov %ax,%fs
    mov %ax,%fs
    mov %ax,%gs


.code32
.balign 4
_APU_Code32:




.code64
.balign 4
_APU_Code64:





.balign 4
_APU_tmp_IDT:
    .word 0
    .word 0,0




_APU_boot_tmp_Stack_end:
ENTRY(_APU_boot_end)