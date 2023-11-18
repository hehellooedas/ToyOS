#include <gate.h>
#include <trap.h>
#include <printk.h>
#include <stddef.h>


void sys_vector_init(void){
    /*  设置系统异常(陷进)中断  */
    set_trap_gate(0, 1, divide_error);
    set_trap_gate(1,1,debug);
    set_intr_gate(2,1,nmi);
    set_system_gate(3, 1, int3);
    set_system_gate(4,1,overflow);
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

    //set_system_gate(SYSTEM_CALL_VECTOR, 7, system_call);
}




/*
在x86_64架构中，函数的参数优先使用寄存器来传递,参数太多才使用栈传递
1、RDI
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
void do_divide_error(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_divide_error(0),ERROR_CODE:%#018x,RSP:%#018x,RIP:%#018x\n",error_code,rsp,*p);
    while(1);
}



void do_debug(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_debug(1),ERROR_CODE:%#018x,RSP:%#018x,RIP:%#018x\n",error_code,rsp,*p);
    while(1);
}



/*  不可屏蔽中断,不是异常  */
void do_nmi(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_dmi(2),ERROR_CODE:%#0181x,RSP:%#0181x,RIP:%#0181x\n",error_code,rsp,*p);
    while(1);
}










void do_int3(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_int3(3),ERROR_CODE:%#0181x,RSP:%#0181x,RIP:%#0181x\n",error_code,rsp,*p);
    while(1);
}


void do_overflow(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_overflow(4),ERROR_CODE:%#0181x,RSP:%#0181x,RIP:%#0181x\n",error_code,rsp,*p);
    while(1);
}


void do_bounds(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_bounds(5),ERROR_CODE:%#0181x,RSP:%#0181x,RIP:%#0181x\n",error_code,rsp,*p);
    while(1);
}


void do_undefined_opcode(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_undefined_opcode(6),ERROR_CODE:%#0181x,RSP:%#0181x,RIP:%#0181x\n",error_code,rsp,*p);
    while(1);
}


void do_dev_not_avaliable(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_dev_not_avaliable(7),ERROR_CODE:%#0181x,RSP:%#0181x,RIP:%#0181x\n",error_code,rsp,*p);
    while(1);
}


void do_double_fault(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_double_fault(8),ERROR_CODE:%#018x,RSP:%#018x,RIP:%#018x\n",error_code,rsp,*p);
    while(1);
}



void do_coprocessor_segmeng_overrun(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_coprocessor_segmeng_overrun(9),ERROR_CODE:%#0181x,RSP:%#0181x,RIP:%#0181x\n",error_code,rsp,*p);
    while(1);
}


void do_segment_not_protection(unsigned long rsp,unsigned long error_code){
    color_printk(RED,BLACK,"segment_not_protection");
    while(1);
}



void do_stack_segment_fault(unsigned long rsp,unsigned long error_code){
    while(1);
}


void do_general_protection(unsigned long rsp,unsigned long error_code){
    color_printk(RED,BLACK,"do_general_protection(13):%d\n",error_code);
    while(1);
}


void do_invalid_TSS(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
    "do_invalid_TSS(10),ERROR_CODE:%0181x,RSP:%0181x,RIP:%0181x\n",error_code,rsp,*p \
    );
    if(error_code & 0x01){
        color_printk(RED,BLACK, \
        "The exception occurred during delivery of an event external to the \
        program,such as an interrupt or an earlier exception.\n"
        );
    }
    if(error_code & 0x02){
        color_printk(RED,BLACK,\
        "Refers to a gate descriptor in the GDT;\n");
    }else{
        color_printk(RED,BLACK,\
        "Refer to a descriptor in the GDT or the currrent LDT;\n");
    }
    if((error_code & 0x02) == 0){
        if(error_code & 0x04){
            color_printk(RED,BLACK,\
            "Refer to a segment or gate descriptor in the LDT;\n");
        }else{
            color_printk(RED,BLACK,\
            "Refer to a descriptor in the current GDT;\n");
        }
    }
    color_printk(RED,BLACK,\
    "Segment Selector Index:%#010x\n",error_code & 0xfff8);
    while(1);
}



void do_page_fault(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    unsigned long cr2 = 0;
    asm volatile ("movq %%cr2,%0":"=r"(cr2)::"memory");
    p = (unsigned long*)(rsp + 0x98);

    color_printk(RED,BLACK,\
    "do_page_fault(14),ERROR_CODE:%#018x,RSP:%#018x,RIP:%#018x\n",error_code,rsp,*p);
    if(!(error_code & 0x01)){  //页不存在引发异常
        color_printk(RED,BLACK,"Page Not-Present\t");
    }
    if(error_code & 0x02){  //写入页引发异常
        color_printk(RED,BLACK,"Write Cause Fault");
    }else{  //读取页引发异常
        color_printk(RED,BLACK,"Read Cause Fault");
    }
    if(error_code & 0x04){
        color_printk(RED,BLACK,"Fault in user(3)\t");
    }else {
        color_printk(RED,BLACK,"Fault in supervisor(0,1,2)\t");
    }
    if(error_code & 0x08){  //页表项保留位引发错误
        color_printk(RED,BLACK,"Reserved Bit Cause Fault\t");
    }

    if(error_code & 0x10){  //获取指令时引发异常
        color_printk(RED,BLACK,"Insturction fetch Cause Fault");
    }
    color_printk(RED,BLACK,"\nCR2:%#018x\n",cr2);
    while(1);
}



void do_x87_FPU_error(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_x87_FPU_error(16),ERROR_CODE:%#0181x,RSP:%#0181x,RIP:%#0181x\n",error_code,rsp,*p);
    while(1);
}



void do_alignment_check(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_alignment_check(17),ERROR_CODE:%#0181x,RSP:%#0181x,RIP:%#0181x\n",error_code,rsp,*p);
    while(1);
}



void do_machine_check(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_machine_check(18),ERROR_CODE:%#0181x,RSP:%#0181x,RIP:%#0181x\n",error_code,rsp,*p);
    while(1);
}



void do_SIMD_exception(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_SIMD_exception(19),ERROR_CODE:%#0181x,RSP:%#0181x,RIP:%#0181x\n",error_code,rsp,*p);
    while(1);
}



void do_virtualization_exception(unsigned long rsp,unsigned long error_code){
    unsigned long* p = NULL;
    p = (unsigned long*)(rsp + 0x98);
    color_printk(RED, BLACK, \
"do_virtualization_exception(20),ERROR_CODE:%#0181x,RSP:%#0181x,RIP:%#0181x\n",error_code,rsp,*p);
    while(1);
}

