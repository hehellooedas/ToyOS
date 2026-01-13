#ifndef __Task_Task_H
#define __Task_Task_H


#include <cpu.h>
#include <lib.h>
#include <list.h>
#include <memory.h>
#include <printk.h>
#include <ptrace.h>




#define STACK_SIZE 32768 // 32K(暂且如此设定)

#define TASK_FILE_MAX 10 // 进程打开的文件超过这个值就会引入动态指针数组

/*  进程状态  */
#define TASK_RUNNING         (1 << 0)   //进程处于可执行状态(正在执行或在就绪队列里)
#define TASK_INTERRUPTIBLE   (1 << 1)   //可中断睡眠态(浅睡眠)被阻塞以等待某些条件达成或资源就位
#define TASK_UNINTERRUPTIBLE (1 << 2)   //不可中断态(对信号不做任何反应)
#define TASK_ZOMBIE          (1 << 3)   //僵尸态(进程已经消亡,但pcb还未释放)
#define TASK_STOPPED         (1 << 4)   //进程停止运行



#define PF_KTHREAD    (1UL << 0)  //内核线程标记
#define NEED_SCHEDULE (1UL << 1) // 标记当前进程是否可被调度
#define PF_VFORK      (1UL << 2) // 在调用exec类函数时,明确是否要为进程再开辟独立的资源空间



#define KERNEL_CS 0x08    // 内核代码段选择子
#define KERNEL_DS 0x10    // 内核数据段选择子
#define USER_CS   0x28    // 用户代码段选择子
#define USER_DS   0x30    // 用户数据段选择子



#define CLONE_VM      (1 << 0)  // 进程间共享虚拟内存
#define CLONE_FS      (1 << 1)  // 进程间共享文件系统信息(打开的文件)
#define CLONE_SIGNAL  (1 << 2)  // 进程间共享信号


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
struct mm_struct {
    pml4t_t *pgd; // 内存页表指针(cr3的值)

    unsigned long start_code, end_code;     // 代码段区域
    unsigned long start_data, end_data;     // 数据段区域
    unsigned long start_rodata, end_rodata; // 只读数据段区域
    unsigned long start_bss, end_bss;
    unsigned long start_brk, end_brk; // 堆区域
    unsigned long start_stack;        // 应用层栈基地址
};



/*  PCB是进程的身份证  */
struct task_struct {
    volatile long state; // 进程状态
    unsigned long flags; // 进程标志(进程/线程/内核线程)
    long
        preempt_count; // preempt_count>0说明该进程持有自旋锁(受保护,不能被调度出去)
    long signal;       // 进程持有的信号
    long cpu_id;       // 该进程绑定哪一个核心

    struct mm_struct *mm; // 内存空间分布结构体
    struct thread_struct *thread; // 进程切换时保留的状态信息(紧跟在pcb后面)
    struct List list;             // 双向链表
    unsigned long addr_limit; // 进程地址空间范围(内核/用户空间)
    /*
    * 0x0000000000000000 - 0x00007fffffffffff属于用户空间
    * 0xffff800000000000 - 0xffffffffffffffff属于内核空间
    */

    long pid;      // 进程ID
    long priority; // 进程优先级
    long virtual_runtime;

    long exit_code;
    struct file *file_struct[TASK_FILE_MAX];
    struct task_struct *next;
    struct task_struct *parent;
};


/*  参与进程调度所必须的信息  */
struct thread_struct {
    unsigned long rsp0; // 内核层栈基地址(在tss里) 一直指向pcb的末尾
    unsigned long rip; // 内核层代码指针(进程切换时的rip)(重要)
    unsigned long
        rsp; // 内核层当前栈指针(进程切换时的rsp) (.rsp~.rsp0是pt_regs)(重要)

    /*  es和ds是不需要保存在tcb里的,因为它会被保存在栈里  */
    unsigned long fs; // FS段寄存器
    unsigned long gs; // GS段寄存器

    unsigned long cr2;        // cr2控制寄存器
    unsigned long trap_nr;    // 产生异常的异常号
    unsigned long error_code; // 异常的错误码
};



/*
占用32KB,起始地址按照32KB对齐
-------------------------
        内核层栈空间
-------------------------
     进程pcb(约等于1KB)
-------------------------
*/
union task_union { // 两个变量共享一个区域(大小为stack变量的大小)
    struct task_struct task;
    struct thread_struct thread;
    unsigned long stack[STACK_SIZE /sizeof(unsigned long)]; // 32KB(视堆栈中元素为8字节变量)
} __attribute__((aligned(8)));




/*
在保护模式下，TSS主要用于硬件级别的任务切换，每个任务都有自己的TSS(通常不用)
在长模式下，TSS主要用于在发生中断或权限级别更改后，更新栈指针
当中断发生时,CPU会查看IDT中对应的中断门/陷进门的属性字段。
若该字段的IST位指定了索引，则将索引对应的IST作为堆栈指针

如果本来就在内核态CPL=0，当发生中断/异常时，不使用rsp0~rsp2，这时候是否换栈由IST决定
rsp0:进入CPL=0时使用(最重要)
rsp1:进入CPL=1时使用(本系统不使用)
rsp2:进入CPL=2时使用(本系统不使用)
*/
struct tss_struct {
    unsigned int reserved0; // 保留,没有特定用途
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
} __attribute__((packed));


extern struct mm_struct init_mm;


/*  初始化为内核进程(填写pcb信息)  */
#define INIT_TASK(task)                                                        \
  {                                                                            \
    .state = TASK_UNINTERRUPTIBLE, .flags = PF_KTHREAD, .mm = &init_mm,        \
    .thread = &init_thread, .addr_limit = 0xffff800000000000, .pid = 0,        \
    .signal = 0, .priority = 2, .cpu_id = 0, .preempt_count = 0,                \
    .virtual_runtime = 0, .file_struct = {0}, .next = &task, .parent = &task,   \
    .exit_code = 0 \
  }



/*
init进程就是以0x118000为开始(pcb)的进程
将init_task_uniox这个全局变量绑定到一个特别的程序段内
*/
extern union task_union init_task_union;

extern struct task_struct *init_task[NR_CPUS];

extern struct mm_struct init_mm;

extern struct thread_struct init_thread;

/*  初始化TSS结构体(用于将TSS信息写入到TSS_Table)  */
#define INIT_TSS                                                               \
  {                                                                            \
    .reserved0 = 0,                                                            \
    .rsp0 = (unsigned long)(init_task_union.stack +                            \
                            STACK_SIZE / sizeof(unsigned long)),               \
    .rsp1 = (unsigned long)(init_task_union.stack +                            \
                            STACK_SIZE / sizeof(unsigned long)),               \
    .rsp2 = (unsigned long)(init_task_union.stack +                            \
                            STACK_SIZE / sizeof(unsigned long)),               \
    .reserved1 = 0, .ist1 = 0xffff800000007c00, .ist2 = 0xffff800000007c00,    \
    .ist3 = 0xffff800000007c00, .ist4 = 0xffff800000007c00,                    \
    .ist5 = 0xffff800000007c00, .ist6 = 0xffff800000007c00,                    \
    .ist7 = 0xffff800000007c00, .reserved2 = 0, .reserved3 = 0,                \
    .iomapbaseaddr = 0                                                         \
  }

extern struct tss_struct init_tss[NR_CPUS];




/*  函数声明  */
void task_init(void);
unsigned long init(unsigned long arg);
int kernel_thread(unsigned long (*fn)(unsigned long), unsigned long arg,
                  unsigned long flags,long priority);
unsigned long do_fork(struct pt_regs *regs, unsigned long clone_flags,
                      unsigned long stack_start, unsigned long stack_size,long priority);
unsigned long do_exit(unsigned long code);

extern void ret_from_intr(void);
extern void ret_system_call(void);
extern void system_call(void);

unsigned long system_call_function(struct pt_regs *regs);
unsigned long do_execve(struct pt_regs *regs,char* name);
void user_level_function();
struct task_struct *get_task(long pid);
void wakeup_process(struct task_struct *task);
unsigned long copy_flags(unsigned long clone_flags, struct task_struct *task);
unsigned long copy_files(unsigned long clone_flags,struct task_struct* task);
void exit_files(struct task_struct* task);
unsigned long copy_mm(unsigned long clone_flags,struct task_struct* task);
void exit_mm(struct task_struct* task);
unsigned long copy_thread(
    unsigned long clone_flags,
    unsigned long stack_start,
    unsigned long stack_size,
    struct task_struct* task,
    struct pt_regs* regs
);
void exit_thread(struct task_struct* task);
struct file* open_exec_file(char* name);

/*  获取当前正在运行的进程的pcb  */
static __attribute__((always_inline)) struct task_struct *get_current() {
    struct task_struct *current = NULL;
    asm volatile("andq %%rsp,%0  \n\t"
                : "=r"(current)
                : "0"(~32767UL)
                : "memory");
    return current;
}
#define running_threaed() get_current()
#define current get_current()

#define GET_CURRENT                                                            \
  "movq %rsp,%rbx     \n\t"                                                    \
  "andq $-32768,%rbx  \n\t"



static __attribute__((always_inline)) void print_pcb_info() {
  struct task_struct *task = current;   //获取pcb
  color_printk(GREEN, BLACK, "print current pcb information:\n");

  /*  当前状态  */
  switch(task->state){
    case TASK_RUNNING:
        color_printk(GREEN, BLACK, "process state:running\n");
      break;
    case TASK_UNINTERRUPTIBLE:
        color_printk(GREEN, BLACK, "process state:uninterruptible\n");
        break;
    case TASK_STOPPED:
      color_printk(GREEN, BLACK, "process state:stopped\n");
      break;
    case TASK_INTERRUPTIBLE:
      color_printk(GREEN, BLACK, "process state:interruptible\n");
      break;
    case TASK_ZOMBIE:
      color_printk(GREEN, BLACK, "process state:zombie\n");
      break;
  }

  /*  所属  */
  switch(task->flags){
    case PF_KTHREAD:
        color_printk(GREEN, BLACK, "process flags:kernel thread\n");
        break;
    }

    color_printk(GREEN, BLACK, "pid:%d\t", task->pid);
    color_printk(GREEN, BLACK, "priority:%d\t",task->priority);
    color_printk(GREEN, BLACK, "virtual_runtime::%d\t", task->virtual_runtime);
    color_printk(GREEN, BLACK, "\n\n");
}

/*
prev in rdi and next in rsi
保存在寄存器里的参数会一直跟随到__switch_to
switch_to为进程切换的前半段
*/
#define switch_to(prev, next)                                                                                                   \
  do {                                                                                                                          \
    asm volatile(                                                                                                               \
        "pushq %%rbp    \n\t" /*栈帧非常重要,必须保存*/                                                               \
        "pushq %%rax    \n\t"                                                                                                   \
        "movq %%rsp,%0  \n\t" /*保存prev进程的栈*/                                                                        \
        "movq %2,%%rsp  \n\t" /*更新rsp为next的栈*/                                                                        \
        "leaq 1f(%%rip),%%rax    \n\t"                                                                                          \
        "movq %%rax,%1  \n\t" /*保存prev进程的rip,方便后续回到1标记的位置*/                                     \
        "pushq %3       \n\t"                                                                                                   \
        "jmp __switch_to \n\t" /*这里使用jmp而不是call,__switch_to函数执行结束后会直接跳到next->thread_rip*/ \
        "1:              \n\t"                                                                                                  \
        "popq %%rax     \n\t"                                                                                                   \
        "popq %%rbp     \n\t"                                                                                                   \
        : "=m"(prev->thread->rsp), "=m"(prev->thread->rip)                                                                      \
        : "m"(next->thread->rsp), "m"(next->thread->rip), "D"(prev), "S"(next)                                                  \
        : "memory");                                                                                                            \
  } while (0)



/*
  切换cr3寄存器的值,切换页表
  必须在内核层地址中执行,也就是在发生调度的时候schedule()
*/
static __attribute__((always_inline))
void switch_mm(struct task_struct *prev,struct task_struct *next) {
    asm volatile("movq %0,%%cr3    \n\t" : : "r"(next->mm->pgd) : "memory");
}

#endif // !__Task_Task_H
