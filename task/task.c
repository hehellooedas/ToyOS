#include <lib.h>
#include <printk.h>
#include <ptrace.h>
#include <gate.h>
#include <string.h>
#include <task.h>


extern char _data;
extern char _rodata;
extern char _erodata;
extern char _stack_start;



void task_init(void) {
    struct task_struct *p = NULL;
    init_mm.pgd = (pml4t_t *)Get_gdt();
    init_mm.start_code = memory_management_struct.start_code;
    init_mm.end_code = memory_management_struct.end_code;
    init_mm.start_data = (unsigned long)&_data;
    init_mm.end_data = memory_management_struct.end_data;
    init_mm.start_rodata = (unsigned long)&_rodata;
    init_mm.end_rodata = (unsigned long)&_erodata;
    init_mm.start_brk = 0;
    init_mm.end_brk = memory_management_struct.end_brk;
    init_mm.start_stack = (unsigned long)&_stack_start;

    wrmsr(0x174, KERNEL_CS);                  // IA_SYSENTER_CS
    wrmsr(0x175, current->thread->rsp0);      // IA32_SYSEXTER_ESP
    wrmsr(0x176, (unsigned long)system_call); // IA32_SYSENTER_RIP


    set_tss64(init_thread.rsp0, init_tss[0].rsp1, init_tss[0].rsp2,
            init_tss[0].ist1, init_tss[0].ist2, init_tss[0].ist3,
            init_tss[0].ist4, init_tss[0].ist5, init_tss[0].ist6,
            init_tss[0].ist7);

    init_tss[0].rsp0 = init_thread.rsp0;

    list_init(&init_task_union.task.list);  //初始化init进程的链表结构
    kernel_thread(init, 10,
                CLONE_FS | CLONE_FILES | CLONE_SIGNAL); // 创建init进程(非内核线程)
    init_task_union.task.state = TASK_RUNNING;

    p = container_of(get_List_next(&current->list), struct task_struct, list);


    switch_to(current, p);
}


/*  init内核线程(0x200000~0x208000)  */
unsigned long init(unsigned long arg) {
    struct pt_regs *regs;

    color_printk(RED, BLACK, "init task is running,arg:%#018x\n", arg);

    current->thread->rip = (unsigned long)ret_system_call;
    current->thread->rsp =
        (unsigned long)current + STACK_SIZE - sizeof(struct pt_regs);
    regs = (struct pt_regs *)current->thread->rsp;

    asm volatile(
        "movq %[stack],%%rsp    \n\t"
        "pushq %2               \n\t" // 确立返回地址ret_system_call
        "jmp do_execute         \n\t" //从do_execute函数返回的时候就会自动进入ret_system_call
        :
        :"D"(regs),[stack] "m"(current->thread->rsp),
            "m"(current->thread->rip)
        : "memory"
    );
    return 1;
}


/*  刚创建的进程通过这个函数恢复现场  */
extern void kernel_thread_func(void);
asm(
    "kernel_thread_func:    \n\t"
    "popq %r15              \n\t"
    "popq %r14              \n\t"
    "popq %r13              \n\t"
    "popq %r12              \n\t"
    "popq %r11              \n\t"
    "popq %r10              \n\t"
    "popq %r9               \n\t"
    "popq %r8               \n\t"
    "popq %rbx              \n\t"
    "popq %rcx              \n\t"
    "popq %rdx              \n\t"
    "popq %rsi              \n\t"
    "popq %rdi              \n\t"
    "popq %rbp              \n\t"
    "popq %rax              \n\t"
    "movq %rax,%ds          \n\t"
    "popq %rax              \n\t"
    "movq %rax,%es          \n\t"
    "popq %rax              \n\t"
    "addq $0x38,%rsp        \n\t" // 7 * 8 = 56 = 0x38

    "movq %rdx,%rdi         \n\t" // rdx记录参数信息
    "callq *%rbx            \n\t" // rbx中记录关键地址信息
    "movq %rax,%rdi         \n\t"
    "callq do_exit          \n\t" //进程运行结束后用do_exit退出
);



int kernel_thread(unsigned long (*fn)(unsigned long), unsigned long arg,
                  unsigned long flags) {
    struct pt_regs regs;
    memset(&regs, 0, sizeof(regs)); //没赋值的变量都为0
    regs.rbx = (unsigned long)fn;  // 记录init重要的执行函数地址信息
    regs.rdx = (unsigned long)arg; // 记录函数的参数信息

    regs.ds = KERNEL_DS;
    regs.es = KERNEL_DS;
    regs.cs = KERNEL_CS;
    regs.ss = KERNEL_DS;
    regs.rflags = (1 << 9);
    regs.rip = (unsigned long)kernel_thread_func;

    return do_fork(&regs, flags, 0, 0);
}



unsigned long do_fork(struct pt_regs *regs, unsigned long clone_flags,
                      unsigned long stack_start, unsigned long stack_size) {
    struct task_struct *task = NULL;
    struct thread_struct *thread = NULL;
    struct Page *p = NULL;

    color_printk(WHITE, BLACK, "alloc_pages,bitmap:%#018x\n",
                *memory_management_struct.bits_map);

    p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped | PG_Active | PG_Kernel);

    color_printk(WHITE, BLACK, "alloc_pages,bitmap:%#018x\n",
                *memory_management_struct.bits_map);

    task = (struct task_struct *)(Phy_To_Virt(p->PHY_address));  //pcb指向新分配的空间首地址

    color_printk(WHITE, BLACK, "struct task_struct address:%#018x\n",
                (unsigned long)task);

    memset(task, 0, sizeof(*task));
    *task = *current; //不同的部分后面可以修改

    list_init(&task->list);

    list_add_to_behind(&init_task_union.task.list, &task->list);

    task->pid++;
    task->state = TASK_UNINTERRUPTIBLE;

    thread = (struct thread_struct *)(task + 1);
    task->thread = thread;

    memcpy((void *)((unsigned long)task + STACK_SIZE - sizeof(struct pt_regs)),
            regs, sizeof(struct pt_regs));

    thread->rsp0 = (unsigned long)task + STACK_SIZE;
    thread->rip = regs->rip;
    thread->rsp = (unsigned long)task + STACK_SIZE - sizeof(struct pt_regs);

    if (!(task->flags & PF_KTHREAD)) {
        /*  如果不是内核层,则设置进程运行的入口为ret_system_call,由此进入用户态  */
        thread->rip = regs->rip = (unsigned long)ret_system_call;
    }
    task->state = TASK_RUNNING;

    return 0;
}


/*  进程切换的下半段  */
void __attribute__((always_inline))
__switch_to(struct task_struct *prev, struct task_struct *next) {
    init_tss[0].rsp0 = next->thread->rsp0; //更新rsp0

    set_tss64(init_tss[0].rsp0, init_tss[0].rsp1, init_tss[0].rsp2,
            init_tss[0].ist1, init_tss[0].ist2, init_tss[0].ist3,
            init_tss[0].ist4, init_tss[0].ist5, init_tss[0].ist6,
            init_tss[0].ist7);  //更新后写入到TSS

    /*
        asm volatile ("movq %%fs,%0 \n\t":"=a"(prev->thread->fs));
        asm volatile ("movq %%gs,%0 \n\t":"=a"(prev->thread->gs));

        asm volatile ("movq %0,%%fs \n\t":"=a"(next->thread->fs));
        asm volatile ("movq %0,%%gs \n\t":"=a"(next->thread->gs));
    */

    /*  相互保存fs和gs段寄存器  */
    asm volatile(
        "movq %%fs,%0       \n\t"
        "movq %%gs,%1       \n\t"
        "movq %2,%%fs       \n\t"
        "movq %3,%%gs       \n\t"
        : "=a"(prev->thread->fs), "=b"(prev->thread->gs)
        : "c"(next->thread->fs), "d"(next->thread->gs)
        : "memory"
    );

    color_printk(WHITE, BLACK, "prev->thread->rsp0:%#-18x\n", prev->thread->rsp0);
    color_printk(WHITE, BLACK, "next->thread->rsp0:%#-18x\n", next->thread->rsp0);
}



unsigned long do_exit(unsigned long code) {
    color_printk(RED, BLACK, "exit task is running,arg:%#018x\n", code);
    while (1);
}


unsigned long do_execute(struct pt_regs *regs) {
    regs->rdx = 0x800000; // rip
    regs->rcx = 0xa00000; // rsp
    regs->rax = 1;
    regs->ds = 0;
    regs->es = 0;
    color_printk(RED, BLACK, "do_execute task is running\n");
    memcpy((void *)0x800000, user_level_function, 1024);
    return 0;
}


void user_level_function() {
    long ret = 0;
    asm volatile(
        "leaq sysexit_return_address(%%rip),%%rdx      \n\t"
        "movq %%rsp,%%rcx   \n\t"
        "sysenter           \n\t"
        "sysexit_return_address: \n\t"
        : "=a"(ret)
        : "0"(1), "D"(__FUNCTION__) // 使用1号系统调用(打印字符串)
        : "memory"
    );
    // color_printk(RED,BLACK,"user_level_function task called
    // sysenter,ret:%ld\n",ret);
    while (1);
}




/*  以下是system call API  */
/*  通过这个函数进入系统调用的实际函数,rax存储系统调用号  */
unsigned long system_call_function(struct pt_regs *regs) {
    return system_call_table[regs->rax](regs);
}

/*  0号系统调用:默认(打印固定字符串)  */
unsigned long default_system_call(struct pt_regs *regs) {
    color_printk(RED, BLACK, "default system call is calling,NR:%#04x\n",
            regs->rax);
    return -1;
}

/*  1号系统调用:打印指定字符串  */
unsigned long sys_printf(struct pt_regs *regs) {
    color_printk(BLACK, WHITE, (char *)(regs->rdi));
    return 1;
}