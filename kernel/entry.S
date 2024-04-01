#include "linkage.h"


/*  这是PCB中几个变量的偏移地址  */
TASK_STATE   =   0x00
TASK_FLAGS   =   0x08
TASK_PREEMPT =   0x10
TASK_SIGNAL  =   0x18

.code64
/*  各个寄存器在栈中的偏移量  */
R15     =   0x00  #跳入函数前最后入栈的寄存器
R14     =   0x08
R13     =   0x10
R12     =   0x18
R11     =   0x20
R10     =   0x28
R9      =   0x30
R8      =   0x38
RBX     =   0x40
RCX     =   0x48
RDX     =   0x50
RSI     =   0x58
RDI     =   0x60
RBP     =   0x68
DS      =   0x70
ES      =   0x78
RAX     =   0x80
FUNC    =   0x88
ERRCODE =   0x90
RIP     =   0x98
CS      =   0xa0
RFLAGS  =   0xa8
#以下寄存器只有在特权级切换的时候才入栈
OLDRSP  =   0xb0
OLDSS   =   0xb8


RESTORE_ALL:
    popq %r15;
    popq %r14;
    popq %r13;
    popq %r12;
    popq %r11; 
    popq %r10; 
    popq %r9; 
    popq %r8; 
    popq %rbx; 
    popq %rcx; 
    popq %rdx; 
    popq %rsi; 
    popq %rdi; 
    popq %rbp; 
    popq %rax;
    movq %rax,%ds; 
    popq %rax; 
    movq %rax,%es; 
    popq %rax; 
    addq $0x10,%rsp; #手动弹出栈中的错误码
    iretq;


#define GET_CURRENT(reg) \
    movq $-32768,reg; \
    andq %rsp,reg


ret_from_exception:

ENTRY(ret_from_intr)
    /*  判断是否有软中断需要解决  */
    movq $-1,%rcx
    testq softirq_status(%rip),%rcx
    jnz softirq_handler
    /*  判断是否进程需要调度  */
    GET_CURRENT(%rbx)
    movq TASK_FLAGS(%rbx),%rcx
    testq $2,%rcx
    jnz reschedule
    jmp RESTORE_ALL


softirq_handler:
    callq do_softirq
    jmp RESTORE_ALL


reschedule:
    callq schedule
    GET_CURRENT(%rbx)
    movq TASK_FLAGS(%rbx),%rcx
    testq $2,%rcx
    jnz reschedule
    jmp RESTORE_ALL
    jmp RESTORE_ALL


ENTRY(divide_error)
    pushq $0
    pushq %rax
    leaq do_divide_error(%rip),%rax
    xchgq %rax,(%rsp)


error_code:
    pushq %rax
    movq %es,%rax
    pushq %rax
    movq %ds,%rax
    pushq %rax
    xorq %rax,%rax

    pushq %rbp
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx 
    pushq %rbx 
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    cld
    movq ERRCODE(%rsp),%rsi #第二个参数
    movq FUNC(%rsp),%rdx
    movq $0x10,%rdi
    movq %rdi,%ds
    movq %rdi,%es
    movq %rsp,%rdi #第一个参数

    callq *%rdx #调用c语言函数处理中断
    jmp ret_from_exception



ENTRY(debug)
    pushq $0
    pushq %rax
    leaq do_debug(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(nmi)
    pushq %rax
    cld;
    pushq %rax;

    pushq %rax
    movq %es,%rax
    pushq %rax
    movq %ds,%rax
    pushq %rax
    xorq %rax,%rax

    pushq %rbp
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %rbx
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13 
    pushq %r14
    pushq %r15 

    movq $0x10,%rdx
    movq %rdx,%es
    movq %rdx,%ds

    movq $0,%rsi
    movq %rsp,%rdi
    callq do_nmi

    callq do_nmi
    jmp RESTORE_ALL


ENTRY(int3)
    pushq $0
    pushq %rax
    leaq do_int3(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(overflow)
    pushq $0
    pushq %rax
    leaq do_overflow(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(bounds)
    pushq $0
    pushq %rax
    leaq do_bounds(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(undefined_opcode)
    pushq $0
    pushq %rax
    leaq do_undefined_opcode(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(dev_not_avaliable)
    pushq $0
    pushq %rax
    leaq do_dev_not_avaliable(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(double_fault)
    pushq $0
    pushq %rax
    leaq do_double_fault(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(coprocessor_segment_overrun)
    pushq $0
    pushq %rax
    leaq do_double_fault(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(invalid_TSS)
    push %rax
    leaq do_invalid_TSS(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(segment_not_protection)
    push %rax
    leaq do_segment_not_protection(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(stack_segment_fault)
    push %rax
    leaq do_stack_segment_fault(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(general_protection)
    push %rax
    leaq do_general_protection(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code



ENTRY(page_fault)
    pushq %rax
    leaq do_page_fault(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code



ENTRY(x87_FPU_error)
    pushq $0
    pushq %rax
    leaq do_x87_FPU_error(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(alignment_check)
    pushq $0
    pushq %rax
    leaq do_alignment_check(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code



ENTRY(machine_check)
    pushq $0
    pushq %rax
    leaq do_machine_check(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code



ENTRY(SIMD_exception)
    pushq $0
    pushq %rax
    leaq do_SIMD_exception(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code


ENTRY(virtualization_exception)
    pushq $0
    pushq %rax
    leaq do_virtualization_exception(%rip),%rax
    xchgq %rax,(%rsp)
    jmp error_code





ENTRY(system_call) #执行sysenter的时候会进入到这里
    sti
    subq $0x38,%rsp
    cld;

    pushq %rax
    movq %es,%rax
    pushq %rax
    movq %ds,%rax
    pushq %rax
    xorq %rax,%rax
    pushq %rbp
    pushq %rdi
    pushq %rsi
    pushq %rdx  #保存rip(sysexit_return_address)
    pushq %rcx  #保存rsp(保存这二者后在sysexit的时候能返回去)
    pushq %rbx
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq $0x10,%rdx
    movq %rdx,%ds
    movq %rdx,%es
    movq %rsp,%rdi  #regs

    callq system_call_function #执行完成后会返回
    # 从用户态程序返回后就往下执行ret_system_call(回到内核态)


ENTRY(ret_system_call)
    movq %rax,0x80(%rsp)  #保存系统调用的返回值rax到regs->rax
    popq %r15;
    popq %r14;
    popq %r13;
    popq %r12;
    popq %r11; 
    popq %r10; 
    popq %r9; 
    popq %r8; 
    popq %rbx; 
    popq %rcx; #上面保存的地址信息就在这里还原了
    popq %rdx; #在do_execute中写入的信息(这两个寄存器非常重要)
    popq %rsi; 
    popq %rdi; 
    popq %rbp; 
    popq %rax;
    movq %rax,%ds; 
    popq %rax; 
    movq %rax,%es; 
    popq %rax; 
    addq $0x38,%rsp; 
    .byte 0x48
    sysexitq
#sysexit只能ring0 -> ring3,跳转时不会进行段描述符检测,速度快
#在使用sysexit之前必须设定好MSR寄存器和rdx,rcx
#IA32_SYSENTER_CS(0x174)
#rdx:被装入rip,rcx:被装入rsp