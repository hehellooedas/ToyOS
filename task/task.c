#include <memory.h>
#include <schedule.h>
#include <lib.h>
#include <printk.h>
#include <ptrace.h>
#include <gate.h>
#include <string.h>
#include <task.h>
#include <control.h>
#include <random.h>
#include <SMP.h>
#include <spinlock.h>


extern char _data;
extern char _rodata;
extern char _erodata;
extern char _stack_start;


struct tss_struct init_tss[NR_CPUS] = {[0 ... (NR_CPUS - 1)] = INIT_TSS};

union task_union init_task_union __attribute__((
    __section__(".data.init_task"))) = {INIT_TASK(init_task_union.task)};

struct task_struct *init_task[NR_CPUS] = {&init_task_union.task, 0};

struct mm_struct init_mm = {0};

struct thread_struct init_thread = {
    .rsp0 = (unsigned long)(init_task_union.stack +
    STACK_SIZE / sizeof(unsigned long)),
    .rsp = (unsigned long)(init_task_union.stack +
    STACK_SIZE / sizeof(unsigned long)),
    .fs = KERNEL_DS,
    .gs = KERNEL_DS,
    .cr2 = 0,
    .trap_nr = 0,
    .error_code = 0
};


system_call_t system_call_table[MAX_SYSTEM_CALL_NR] = {
    [0] = default_system_call,
    [1] = sys_printf,
    [2 ...(MAX_SYSTEM_CALL_NR - 1)] = default_system_call
};




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
    if(SMP_cpu_id() == 0){

        list_init(&init_task_union.task.list);  //初始化init进程的链表结构

        init_task_union.task.state = TASK_RUNNING;
        init_task_union.task.preempt_count = 0;
        init_task_union.task.cpu_id = 0;
    }


    wrmsr(0x174, KERNEL_CS);                  // IA_SYSENTER_CS
    wrmsr(0x175, current->thread->rsp0);      // IA32_SYSEXTER_ESP
    wrmsr(0x176, (unsigned long)system_call); // IA32_SYSENTER_RIP

    init_tss[SMP_cpu_id()].rsp0 = init_thread.rsp0;
    //color_printk(YELLOW,BLACK ,"currect->thread->rsp0=%#lx\n",current->thread->rsp0 );
    color_printk(YELLOW,BLACK ,"current=%#lx\n",current );
    kernel_thread(init, 10,
                CLONE_FS | CLONE_FILES | CLONE_SIGNAL); // 创建init进程(非内核线程)

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



int kernel_thread(unsigned long (*fn)(unsigned long),
                  unsigned long arg,
                  unsigned long flags
){
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



unsigned long do_fork(
    struct pt_regs *regs,
    unsigned long clone_flags,
    unsigned long stack_start,
    unsigned long stack_size
){
    struct task_struct *task = NULL;
    struct thread_struct *thread = NULL;
    struct Page *p = NULL;

    color_printk(WHITE, BLACK, "alloc_pages,bitmap:%#018x\n",
                *memory_management_struct.bits_map);

    p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped | PG_Kernel);

    color_printk(WHITE, BLACK, "alloc_pages,bitmap:%#018x\n",
                *memory_management_struct.bits_map);

    task = (struct task_struct *)(Phy_To_Virt(p->PHY_address));  //pcb指向新分配页的首地址

    color_printk(WHITE, BLACK, "struct task_struct address:%#018x\n",
                (unsigned long)task);

    memset(task, 0, sizeof(*task));
    *task = *current; //不同的部分后面可以修改

    list_init(&task->list);


    task->pid++;
    task->state = TASK_UNINTERRUPTIBLE;
    task->priority = 2;
    task->preempt_count = 0;
    task->cpu_id = SMP_cpu_id();

    thread = (struct thread_struct *)(task + 1);
    task->thread = thread;

    memcpy((void *)((unsigned long)task + STACK_SIZE - sizeof(struct pt_regs)),
            regs, sizeof(struct pt_regs));

    thread->rsp0 = (unsigned long)task + STACK_SIZE;
    thread->rip = regs->rip;
    thread->rsp = (unsigned long)task + STACK_SIZE - sizeof(struct pt_regs);
    thread->fs = KERNEL_DS;
    thread->gs = KERNEL_DS;

    if (!(task->flags & PF_KTHREAD)) {
        /*  如果不是内核层,则设置进程运行的入口为ret_system_call,由此进入用户态  */
        thread->rip = regs->rip = (unsigned long)ret_system_call;
    }
    task->state = TASK_RUNNING;

    insert_task_queue(task);    //把创建好的新进程pcb插入到当前核心对应的调度队列中

    return 0;
}




/*  进程切换的下半段  */
void __attribute__((always_inline))
__switch_to(struct task_struct *prev, struct task_struct *next) {
    unsigned int color = 0;
    init_tss[SMP_cpu_id()].rsp0 = next->thread->rsp0; //更新rsp0

    set_tss64((unsigned int*)&init_tss[SMP_cpu_id()],init_tss[SMP_cpu_id()].rsp0, init_tss[SMP_cpu_id()].rsp1, init_tss[SMP_cpu_id()].rsp2,init_tss[SMP_cpu_id()].ist1, init_tss[SMP_cpu_id()].ist2, init_tss[SMP_cpu_id()].ist3,init_tss[SMP_cpu_id()].ist4, init_tss[SMP_cpu_id()].ist5, init_tss[SMP_cpu_id()].ist6,init_tss[SMP_cpu_id()].ist7);  //更新后写入到TSS

    wrmsr(0x175,next->thread->rsp0 );
    if(SMP_cpu_id() == 0){
        color = WHITE;
    }else{
        color = YELLOW;
    }


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

    color_printk(color, BLACK, "prev->thread->rsp0:%#lx\tnext->thread->rsp0:%#lx\tCPU:%d\n", prev->thread->rsp0,next->thread->rsp0,SMP_cpu_id());
}



unsigned long do_exit(unsigned long code) {
    color_printk(RED, BLACK, "exit task is running,arg:%#018x\n", code);
    while (1);
}




unsigned long do_execute(struct pt_regs *regs) {
    unsigned long addr = 0x800000;
    unsigned long* tmp;
    unsigned long* virtual = NULL;
    struct Page* p = NULL;


    regs->rdx = 0x800000; // rip
    regs->rcx = 0xa00000; // rsp
    regs->rax = 1;
    regs->ds = 0;
    regs->es = 0;
    color_printk(RED, BLACK, "do_execute task is running\n");

    Global_CR3 = Get_gdt();
    tmp = Phy_To_Virt((unsigned long*)((unsigned long)Global_CR3 & (~0xfffUL)) + ((addr >> PAGE_GDT_SHIFT) & 0x1ff));
    virtual = kmalloc(PAGE_4K_SIZE,0);
    set_pml4t(tmp,mk_pml4t(Virt_To_Phy(virtual),PAGE_USER_GDT ) );

    tmp = Phy_To_Virt((unsigned long*)(*tmp & (~0xfffUL)) + ((addr >> PAGE_1G_SHIFT) & 0x1ff));
    virtual = kmalloc(PAGE_4K_SIZE,0);
    set_pdpt(tmp,mk_pdpt(Virt_To_Phy(virtual),PAGE_USER_Dir ) );

    tmp = Phy_To_Virt((unsigned long*)(*tmp & (~0xfffUL)) + ((addr >> PAGE_2M_SHIFT) & 0x1ff));
    p = alloc_pages(ZONE_NORMAL,1,PG_PTable_Maped);
    set_pdt(tmp,mk_pdt(p->PHY_address,PAGE_USER_Page ) );

    flush_tlb();

    if(!(current->flags & PF_KTHREAD)){
        current->addr_limit = 0xffff800000000000;
    }

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
    //print_cr0_info();无法在用户态执行该函数
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