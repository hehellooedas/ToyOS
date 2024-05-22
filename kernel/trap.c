#include <gate.h>
#include <stddef.h>
#include <trap.h>
#include <SMP.h>



void sys_vector_init(void) {
    /*
     * 设置系统异常(陷进)中断
     * 0~0x1f中断为CPU内部中断
     */
    set_trap_gate(0, 1, divide_error);
    set_trap_gate(1, 1, debug);
    set_intr_gate(2, 1, nmi);
    set_system_gate(3, 1, int3);
    set_system_gate(4, 1, overflow);
    set_system_gate(5, 1, bounds);
    set_trap_gate(6, 1, undefined_opcode);
    set_trap_gate(7, 1, dev_not_avaliable);
    set_trap_gate(8, 1, double_fault);
    set_trap_gate(9, 1, coprocessor_segment_overrun);
    set_trap_gate(10, 1, invalid_TSS);
    set_trap_gate(11, 1, segment_not_protection);
    set_trap_gate(12, 1, stack_segment_fault);
    set_trap_gate(13, 1, general_protection);
    set_trap_gate(14, 1, page_fault);
    /*  第15号中断为intel保留  */
    set_trap_gate(16, 1, x87_FPU_error);
    set_trap_gate(17, 1, alignment_check);
    set_trap_gate(18, 1, machine_check);
    set_trap_gate(19, 1, SIMD_exception);
    set_trap_gate(20, 1, virtualization_exception);
    /*  21~31号中断为Intel保留  */

    // set_system_gate(SYSTEM_CALL_VECTOR, 7, system_call);
}

/*
在x86_64架构中，函数的参数优先使用寄存器来传递,参数太多才使用栈传递
0、保存函数的返回值
1、RDI(从左到右的顺序)
2、RSI
3、RDX
4、RCX
5、R8
6、R9
pushq
*/

/*
触发中断程序时栈的变化
---------
   SS
---------
   RSP
特权级变化时才保存以上信息
---------
  RFLAGS
---------
   CS
---------
   RIP
---------
Error Code
---------
程序员手动保存其他寄存器
---------
异常处理结束后弹出手动保存的寄存器(包括错误码信息)
---------

*/



/*  除0异常  */
void do_divide_error(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_divide_error,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_debug(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_debug,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



/*  不可屏蔽中断,不是异常  */
void do_nmi(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_nmi,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_int3(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_int3,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}




void do_overflow(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_overflow,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_bounds(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_bounds,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_undefined_opcode(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_undefined_opcodeERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_dev_not_avaliable(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_dev_not_avaliable,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_double_fault(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_double_fault,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_coprocessor_segmeng_overrun(struct pt_regs* regs,
                                    unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_coprocessor_segmeng_overrun,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_segment_not_protection(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_segment_not_protection,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_stack_segment_fault(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_stack_segment_fault,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_general_protection(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_general_protection,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}




void do_invalid_TSS(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_invalid_TSS,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);

    if (error_code & 0x01) {
    color_printk(
        RED, BLACK,
        "The exception occurred during delivery of an event external to the \
        program,such as an interrupt or an earlier exception.\n");
    }
    if (error_code & 0x02) {
    color_printk(RED, BLACK, "Refers to a gate descriptor in the GDT;\n");
    } else {
    color_printk(RED, BLACK,
                    "Refer to a descriptor in the GDT or the currrent LDT;\n");
    }
    if ((error_code & 0x02) == 0) {
    if (error_code & 0x04) {
        color_printk(RED, BLACK,
                    "Refer to a segment or gate descriptor in the LDT;\n");
    } else {
        color_printk(RED, BLACK, "Refer to a descriptor in the current GDT;\n");
    }
    }
    color_printk(RED, BLACK, "Segment Selector Index:%#lx\n",
                error_code & 0xfff8);
    while (1)
        hlt();
}



void do_page_fault(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(&error_code,0,3);
    unsigned long cr2 = 0;
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_page_fault,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    if (!(error_code & 0x01)) { // 页不存在引发异常
    color_printk(RED, BLACK, "Page Not-Present\t");
    }
    if (error_code & 0x02) { // 写入页引发异常
    color_printk(RED, BLACK, "Write Cause Fault");
    } else { // 读取页引发异常
    color_printk(RED, BLACK, "Read Cause Fault");
    }
    if (error_code & 0x04) {
    color_printk(RED, BLACK, "Fault in user(3)\t");
    } else {
    color_printk(RED, BLACK, "Fault in supervisor(0,1,2)\t");
    }
    if (error_code & 0x08) { // 页表项保留位引发错误
    color_printk(RED, BLACK, "Reserved Bit Cause Fault\t");
    }

    if (error_code & 0x10) { // 获取指令时引发异常
    color_printk(RED, BLACK, "Insturction fetch Cause Fault");
    }
    color_printk(RED, BLACK, "\nCR2:%#lx\n", cr2);
    while (1)
        hlt();
}



void do_x87_FPU_error(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_x87_FPU_error,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_alignment_check(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_alignment_check,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_machine_check(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_machine_check,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_SIMD_exception(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_SIMD_exception,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}



void do_virtualization_exception(struct pt_regs* regs, unsigned long error_code) {
    __builtin_prefetch(regs,0,1);
    color_printk(RED,BLACK,"do_virtualization_exception,ERROR_CODE:%d,CPU:%d,PID:%d\n",error_code,SMP_cpu_id(),current->pid);
    print_regs(regs);
    while (1)
        hlt();
}




void backtrace(struct pt_regs* regs){
    unsigned long* rbp = (unsigned long*)regs->rbp;
    unsigned long ret_address = regs->rip;

}
