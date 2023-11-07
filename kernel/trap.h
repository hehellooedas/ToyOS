#ifndef __KERNEL_TRAP_H
#define __KERNEL_TRAP_H



/*  中断处理函数(在entry.S中定义)  */
extern void divide_error(void);       //除法错误
extern void debug(void);              //调试异常
extern void nmi(void);                //NMI中断
extern void int3(void);               //int3中断
extern void overflow(void);           //into中断
extern void bounds(void);             //bound中断
extern void undefined_opcode(void);   //未定义的机器码(用于软件测试)
extern void dev_not_avaliable(void);  //设备不存在(device)
extern void double_fault(void);       //双重错误
extern void coprocessor_segment_overrun(void); //协处理器段越界
extern void invalid_TSS(void);        //无效的TSS段
extern void segment_not_protection(void);  //段不存在
extern void stack_segment_fault(void);//SS段错误
extern void general_protection(void); //通用性保护异常
extern void page_fault(void);         //缺页异常
extern void x87_FPU_error(void);      //x87FPU错误
extern void alignment_check(void);    //对齐检测
extern void machine_check(void);      //机器检测
extern void SIMD_exception(void);     //SIMD浮点异常
extern void virtualization_exception(void);  //虚拟化异常


/*  中断描述符表初始化  */
void sys_vector_init(void);



/*  c版中断处理  */
void do_divide_error(unsigned long rsp,unsigned long error_code);
void do_debug(unsigned long rsp,unsigned long error_code);
void do_nmi(unsigned long rsp,unsigned long error_code);
void do_int3(unsigned long rsp,unsigned long error_code);
void do_overflow(unsigned long rsp,unsigned long error_code);
void do_bounds(unsigned long rsp,unsigned long error_code);
void do_undefined_opcode(unsigned long rsp,unsigned long error_code);
void do_dev_not_avaliable(unsigned long rsp,unsigned long error_code);
void do_double_fault(unsigned long rsp,unsigned long error_code);
void do_coprocessor_segmeng_overrun(unsigned long rsp,unsigned long error_code);
void do_invalid_TSS(unsigned long rsp,unsigned long error_code);
void do_segment_not_protection(unsigned long rsp,unsigned long error_code);
void do_stack_segment_fault(unsigned long rsp,unsigned long error_code);
void do_general_protection(unsigned long rsp,unsigned long error_code);
void do_page_fault(unsigned long rsp,unsigned long error_code);
void do_x87_FPU_error(unsigned long rsp,unsigned long error_code);
void do_alignment_check(unsigned long rsp,unsigned long error_code);
void do_machine_check(unsigned long rsp,unsigned long error_code);
void do_SIMD_exception(unsigned long rsp,unsigned long error_code);
void do_virtualization_exception(unsigned long rsp,unsigned long error_code);

#endif // !__KERNEL_TRAP_H



