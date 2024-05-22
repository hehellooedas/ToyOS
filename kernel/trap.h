#ifndef __KERNEL_TRAP_H
#define __KERNEL_TRAP_H


#include <printk.h>
#include <ptrace.h>


/*  中断处理函数(在entry.S中定义)  */
extern void divide_error(void);                 //除法错误
extern void debug(void);                        //调试异常
extern void nmi(void);                          //NMI中断
extern void int3(void);                         //int3中断
extern void overflow(void);                     //into中断
extern void bounds(void);                       //bound中断
extern void undefined_opcode(void);             //未定义的机器码(用于软件测试)
extern void dev_not_avaliable(void);            //设备不存在(device)
extern void double_fault(void);                 //双重错误
extern void coprocessor_segment_overrun(void);  //协处理器段越界
extern void invalid_TSS(void);                  //无效的TSS段
extern void segment_not_protection(void);       //段不存在
extern void stack_segment_fault(void);          //SS段错误
extern void general_protection(void);           //通用性保护异常
extern void page_fault(void);                   //缺页异常
extern void x87_FPU_error(void);                //x87FPU错误
extern void alignment_check(void);              //对齐检测
extern void machine_check(void);                //机器检测
extern void SIMD_exception(void);               //SIMD浮点异常
extern void virtualization_exception(void);     //虚拟化异常


/*  中断描述符表初始化  */
void sys_vector_init(void);



/*  c版中断处理  */
void do_divide_error(struct pt_regs* regs,unsigned long error_code);
void do_debug(struct pt_regs* regs,unsigned long error_code);
void do_nmi(struct pt_regs* regs,unsigned long error_code);
void do_int3(struct pt_regs* regs,unsigned long error_code);
void do_overflow(struct pt_regs* regs,unsigned long error_code);
void do_bounds(struct pt_regs* regs,unsigned long error_code);
void do_undefined_opcode(struct pt_regs* regs,unsigned long error_code);
void do_dev_not_avaliable(struct pt_regs* regs,unsigned long error_code);
void do_double_fault(struct pt_regs* regs,unsigned long error_code);
void do_coprocessor_segmeng_overrun(struct pt_regs* regs,unsigned long error_code);
void do_invalid_TSS(struct pt_regs* regs,unsigned long error_code);
void do_segment_not_protection(struct pt_regs* regs,unsigned long error_code);
void do_stack_segment_fault(struct pt_regs* regs,unsigned long error_code);
void do_general_protection(struct pt_regs* regs,unsigned long error_code);
void do_page_fault(struct pt_regs* regs,unsigned long error_code);
void do_x87_FPU_error(struct pt_regs* regs,unsigned long error_code);
void do_alignment_check(struct pt_regs* regs,unsigned long error_code);
void do_machine_check(struct pt_regs* regs,unsigned long error_code);
void do_SIMD_exception(struct pt_regs* regs,unsigned long error_code);
void do_virtualization_exception(struct pt_regs* regs,unsigned long error_code);




static __attribute__((always_inline))
void print_regs(struct pt_regs* regs)
{
    /*  打印段寄存器信息  */
    color_printk(RED,BLACK,"CS:%#lx,DS:%#lx,ES:%#lx,SS:%#lx\n",regs->cs,regs->ds,regs->es,regs->ss);

    /*  打印特殊通用寄存器信息  */
    color_printk(RED,BLACK ,"RIP:%#lx,RSP:%lx,RBP:%#lx,rflags:%#lx\n",regs->rip,regs->rsp,regs->rbp,regs->rflags );

    /*  打印通用寄存器信息  */
    color_printk(RED,BLACK,"RAX:%#lx,RBX:%lx,RCX:%lx,RDX:%lx,RSI:%lx,RDI:%lx\n",regs->rax,regs->rbx,regs->rcx,regs->rdx,regs->rsi,regs->rdi);

    /*  打印其他通用寄存器信息  */
    color_printk(RED,BLACK,"R8:%#lx,R9:%lx,R10:%#lx,R11:%lx,R12:%lx,R13:%#lx,R14:%lx,R15:%lx\n",regs->r8,regs->r9,regs->r10,regs->r11,regs->r12,regs->r13,regs->r14,regs->r15);

}


#endif // !__KERNEL_TRAP_H



