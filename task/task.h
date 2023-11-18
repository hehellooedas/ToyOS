#ifndef __Task_Task_H
#define __Task_Task_H

#include <cpu.h>
#include <list.h>
#include <printk.h>
#include <memory.h>
#include <ptrace.h>
#include <lib.h>


#define STACK_SIZE 32768  //32K

/*  进程状态  */
#define TASK_RUNNING    (1 << 0)      //运行态
#define TASK_INTERRUPTIBLE   (1 << 1)  //可以响应中断的等待态
#define TASK_UNINTERRUPTIBLE (1 << 2)  //不响应中断的等待态
#define TASK_ZOMBIE     (1 << 3)      //僵尸进程
#define TASK_STOPPED    (1 << 4)      //进程的执行暂停

#define PF_KTHREAD  (1 << 0)


#define KERNEL_CS   0x08  //内核代码段选择子
#define KERNEL_DS   0x10  //内核数据段选择子
#define USER_CS     0x28  //用户代码段选择子
#define USER_DS     0x30  //用户数据段选择子


#define CLONE_FS        (1 << 0)
#define CLONE_FILES     (1 << 1)
#define CLONE_SIGNAL    (1 << 2)


extern char _text;
extern char _etext;
extern char _data;
extern char _edata;
extern char _rodata;
extern char _erodata;
extern char _bss;
extern char _ebss;
extern char _end;




/*  描述进程的页表信息和各程序段的信息  */
struct mm_struct{
    pml4t_t* pgd;   //内存页表指针(cr3的值)

    unsigned long start_code,end_code;     //代码段区域
    unsigned long start_data,end_data;     //数据段区域
    unsigned long start_rodata,end_rodata; //只读数据段区域
    unsigned long start_brk,end_brk;       //堆区域
    unsigned long start_stack;             //应用层栈基地址
};




/*  PCB是进程的身份证  */
struct task_struct{
    struct   List       list;    //双向链表
    volatile long       state;   //进程状态(保证实时状态)
    unsigned long       flags;   //进程标志

    struct mm_struct*   mm;       //内存空间分布结构体
    struct thread_struct* thread; //进程切换时保留的状态信息
    unsigned long addr_limit;     //进程地址空间范围(内核/用户空间)

    long pid;       //进程ID
    long counter;   //时间片
    long signal;    //进程持有的信号
    long priority;  //进程优先级
};



/*
占用32KB,起始地址按照32KB对齐
-------------------------
        内核层栈空间
-------------------------
     进程pcb(约等于1KB)
-------------------------
*/
union task_union{  //两个变量共享一个区域
    struct task_struct task;
    unsigned long stack[STACK_SIZE / sizeof(unsigned long)];  //32KB
}__attribute__((aligned(8)));





/*  参与进程调度所必须的信息  */
struct thread_struct{
    unsigned long rsp0;   //内核层栈基地址(tss里)
    unsigned long rip;    //内核层代码指针(进程切换时的rip)
    unsigned long rsp;    //内核层当前栈指针(进程切换时的rsp)

    /*  es和ds是不需要保存在tcb里的,因为它会被保存在栈里  */
    unsigned long fs;     //FS段寄存器
    unsigned long gs;     //GS段寄存器

    unsigned long cr2;       //cr2控制寄存器
    unsigned long trap_nr;   //产生异常的异常号
    unsigned long error_code;//异常的错误码
};




struct tss_struct{
    unsigned int  reserved0;
    unsigned long rsp0;
    unsigned long rsp1;
    unsigned long rsp2;
    unsigned long reserved1;
    unsigned long ist1;
    unsigned long ist2;
    unsigned long ist3;
    unsigned long ist4;
    unsigned long ist5;
    unsigned long ist6;
    unsigned long ist7;
    unsigned long reserved2;
    unsigned short reserved3;
    unsigned short iomapbaseaddr;
}__attribute__((packed));




struct mm_struct init_mm;
struct thread_struct init_thread;


#define INIT_TASK(tsk)	                \
{			                            \
	.state = TASK_UNINTERRUPTIBLE,		\
	.flags = PF_KTHREAD,		        \
	.mm = &init_mm,			            \
	.thread = &init_thread,		        \
	.addr_limit = 0xffff800000000000,	\
	.pid = 0,			                \
	.counter = 1,		                \
	.signal = 0,		                \
	.priority = 0		                \
}

/*  init进程就是以0x118000为开始(pcb)的进程  */
union task_union init_task_union __attribute__((__section__ (".data.init_task"))) = {INIT_TASK(init_task_union.task)};

struct task_struct *init_task[NR_CPUS] = {&init_task_union.task,0};

struct mm_struct init_mm = {0};

struct thread_struct init_thread = 
{
	.rsp0 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),
	.rsp = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),
	.fs = KERNEL_DS,
	.gs = KERNEL_DS,
	.cr2 = 0,
	.trap_nr = 0,
	.error_code = 0
};





#define INIT_TSS \
{	.reserved0 = 0,	 \
	.rsp0 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),	\
	.rsp1 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),	\
	.rsp2 = (unsigned long)(init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),	\
	.reserved1 = 0,	 \
	.ist1 = 0xffff800000007c00,	\
	.ist2 = 0xffff800000007c00,	\
	.ist3 = 0xffff800000007c00,	\
	.ist4 = 0xffff800000007c00,	\
	.ist5 = 0xffff800000007c00,	\
	.ist6 = 0xffff800000007c00,	\
	.ist7 = 0xffff800000007c00,	\
	.reserved2 = 0,	\
	.reserved3 = 0,	\
	.iomapbaseaddr = 0	\
}

struct tss_struct init_tss[NR_CPUS] = { [0 ... NR_CPUS-1] = INIT_TSS };




/*  函数声明  */
void task_init(void);
unsigned long init(unsigned long arg);
int kernel_thread(unsigned long (*fn)(unsigned long),unsigned long arg,unsigned long flags);unsigned long do_fork(struct pt_regs* regs,unsigned long clone_flags,unsigned long stack_start,unsigned long stack_size);
unsigned long do_exit(unsigned long code);
extern void ret_from_intr(void);



/*  获取当前正在运行的进程的pcb  */
static __attribute__((always_inline))
struct task_struct* get_current()
{
    struct task_struct* current = NULL;
    asm volatile (
        "andq %%rsp,%0  \n\t"
        :"=r"(current)
        :"0"(~32767UL)
        :"memory"
    );
    return current;
}
#define running_threaed() get_current()
#define current get_current()

#define GET_CURRENT     \
    "movq %rsp,%rbx     \n\t"   \
    "andq $-32769,%rbx  \n\t"



#define switch_to(prev,next)       \
do{                                \
    asm volatile (                  \
        "pushq %%rbp    \n\t"       \
        "pushq %%rax    \n\t"       \
        "movq %%rsp,%0  \n\t"       \
        "movq %2,%%rsp  \n\t"       \
        "leaq 1f(%%rip),%%rax    \n\t"      \
        "movq %%rax,%1  \n\t"       \
        "pushq %3       \n\t"       \
        "jmp __switch_to \n\t"      \
        "1:              \n\t"      \
        "popq %%rax     \n\t"       \
        "popq %%rbp     \n\t"       \
        :"=m"(prev->thread->rsp),"=m"(prev->thread->rip)    \
        :"m"(next->thread->rsp),"m"(next->thread->rip),"D"(prev),"S"(next)      \
        :"memory"                   \
    );                              \
}while(0)




#endif // !__Task_Task_H
