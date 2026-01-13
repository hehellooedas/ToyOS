#include <errno.h>
#include <fcntl.h>
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
#include <interrupt.h>
#include <fat32.h>
#include <log.h>
#include <timer.h>
#include <sys.h>
#include <VFS.h>
#include <disk.h>


extern char _data;
extern char _rodata;
extern char _erodata;
extern char _stack_start;


static unsigned long global_pid = 0;

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



unsigned long new_process(unsigned long arg);

void task_init(void) {
    /*
        确保系统创建出来的子进程始终拥有相同的内核层地址空间
        率先把内核层地址空间在顶层页表pml4t_t中映射好
    */
    unsigned long *tmp = NULL,*vaddr = NULL;
    vaddr = (unsigned long*)Phy_To_Virt((unsigned long)Get_gdt() & (~0xfffUL));
    /*
        移除早期启动阶段的低地址恒等映射
        子进程共享内核高半区地址,低半区是由每个进程各自建立的用户空间,不应该默认就带一份启动时的映射
    */
    *vaddr = 0UL;
    for(int i=256;i<512;i++){
        tmp = vaddr + i;
        if(*tmp == 0){
            unsigned long* virtual = kmalloc(PAGE_4K_SIZE,0);
            memset(virtual,0,PAGE_4K_SIZE);
            set_pml4t(tmp,mk_pml4t(Virt_To_Phy(virtual),PAGE_KERNEL_GDT));
        }
    }

    init_mm.pgd = (pml4t_t *)Get_gdt();
    init_mm.start_code = memory_management_struct.start_code;
    init_mm.end_code = memory_management_struct.end_code;
    init_mm.start_data = (unsigned long)&_data;
    init_mm.end_data = memory_management_struct.end_data;

    init_mm.start_rodata = (unsigned long)&_rodata;
    init_mm.end_rodata = memory_management_struct.end_rodata;

    init_mm.start_bss = (unsigned long)&_bss;
    init_mm.end_bss = (unsigned long)&_ebss;
    init_mm.start_brk = memory_management_struct.start_brk;
    init_mm.end_brk = current->addr_limit;
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
                CLONE_FS | CLONE_SIGNAL,5); // 创建init进程(非内核线程)


}






/*  init内核线程(0x200000~0x208000)  */
unsigned long init(unsigned long arg) {
    struct pt_regs *regs;
    Disk1_FAT32_FS_init();

    //color_printk(RED, BLACK, "init task is running,arg:%#018x\n", arg);


    current->thread->rip = (unsigned long)ret_system_call;
    current->thread->rsp =
        (unsigned long)current + STACK_SIZE - sizeof(struct pt_regs);
    regs = (struct pt_regs *)current->thread->rsp;

    current->thread->fs = USER_DS;
    current->thread->gs = USER_DS;
    current->flags &= ~PF_KTHREAD;

    asm volatile(
        "movq %[stack],%%rsp    \n\t"
        "pushq %2               \n\t" // 确立返回地址ret_system_call
        "jmp do_execve         \n\t" //从do_execute函数返回的时候就会自动进入ret_system_call
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
                  unsigned long flags,long priority
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

    return do_fork(&regs, flags|CLONE_VM, 0, 0,priority);
}



unsigned long do_fork(
    struct pt_regs *regs,
    unsigned long clone_flags,
    unsigned long stack_start,
    unsigned long stack_size,long priority
){
    int retval = 0;
    struct task_struct *task = NULL;
    
    struct thread_struct *thread = NULL;
    //struct Page *p = NULL;

    //color_printk(WHITE, BLACK, "alloc_pages,bitmap:%#018x\n",*memory_management_struct.bits_map);

    //p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped | PG_Kernel);

    //color_printk(WHITE, BLACK, "alloc_pages,bitmap:%#018x\n",
    //            *memory_management_struct.bits_map);

    //task = (struct task_struct *)(Phy_To_Virt(p->PHY_address));  //pcb指向新分配页的首地址

    task = (struct task_struct*)kmalloc(STACK_SIZE,0);
    if(task == NULL){
        retval = -EAGAIN;
        goto alloc_copy_task_fail;
    }
    __builtin_prefetch(task,1,3);
    color_printk(WHITE, BLACK, "struct task_struct address:%#018x\n",
                (unsigned long)task);

    memset(task, 0, sizeof(*task));
    memcpy(task,current,sizeof(struct task_struct)); //不同的部分后面可以修改

    list_init(&task->list);


    task->pid = global_pid++;
    task->state = TASK_UNINTERRUPTIBLE;
    task->priority = priority;
    task->preempt_count = 0;
    task->cpu_id = SMP_cpu_id();

    task->next = init_task_union.task.next;
    task->parent = current;

    retval = -ENOMEM;
    // copy flags
    if(copy_flags(clone_flags,task)){
        goto copy_flags_fail;
    }
    // copy mm
    if(copy_mm(clone_flags,task)){
        goto copy_mm_fail;
    }
    // copy files
    if(copy_files(clone_flags,task)){
        goto copy_files_fail;
    }
    // copy thread
    if(copy_thread(clone_flags,stack_start,stack_size,task,regs)){
        goto copy_thread_fail;
    }
    // 返回fork出来新进程的pid
    retval = task->pid;
    /*
        唤醒fork出来的新进程
        设置state为运行态
        插入到就绪队列
    */
    wakeup_process(task);



fork_ok:
    return retval;
copy_thread_fail:
    exit_thread(task);
copy_files_fail:
    exit_files(task);
copy_mm_fail:
    exit_mm(task);


    //复制标志失败
copy_flags_fail:
    // 为task分配空间失败
alloc_copy_task_fail:
    kfree(task);
    return retval;
}




/*  进程切换的下半段  */
void __attribute__((always_inline))
__switch_to(struct task_struct *prev, struct task_struct *next) {
    long cpu_id = SMP_cpu_id();
    unsigned int color = 0;
    init_tss[cpu_id].rsp0 = next->thread->rsp0; //更新rsp0

    set_tss64((unsigned int*)&init_tss[cpu_id],init_tss[cpu_id].rsp0, init_tss[cpu_id].rsp1, init_tss[cpu_id].rsp2,init_tss[cpu_id].ist1, init_tss[cpu_id].ist2, init_tss[cpu_id].ist3,init_tss[cpu_id].ist4, init_tss[cpu_id].ist5, init_tss[cpu_id].ist6,init_tss[cpu_id].ist7);  //更新后写入到TSS

    wrmsr(0x175,next->thread->rsp0 );
    if(cpu_id == 0){
        color = WHITE;
    }else if (cpu_id == 3) {
        color = BLUE;
    }
    else{
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
    log_to_screen(INFO,"CPU:%d prev->thread->rsp0:%#lx\tnext->thread->rsp0:%#lx\t",SMP_cpu_id(),prev->thread->rsp0,next->thread->rsp0);

}



unsigned long do_exit(unsigned long exit_code) {
    color_printk(RED, BLACK, "exit task is running,arg:%#018x\n", exit_code);
    struct task_struct* task = current;
    cli();
    task->exit_code = exit_code;
    task->state = TASK_ZOMBIE;
    sti();
    while (1);
}




unsigned long do_execve(
    struct pt_regs *regs,
    char* name
) {
    unsigned long code_start_address = 0x800000;
    unsigned long stack_start_address = 0xa00000;
    unsigned long *tmp;
    unsigned long *virtual = NULL;
    struct Page *p = NULL;
    struct file* filep = NULL;
    unsigned long retval = 0;
    long pos = 0;

    regs->r10 = 0x800000; // rip
    regs->r11 = 0xa00000; // rsp
    regs->rax = 1;
    regs->ds = USER_DS;
    regs->es = USER_DS;
    regs->ss = USER_DS;
    regs->cs = USER_CS;
    color_printk(RED, BLACK, "do_execve task is running\n");

    //与父进程共享地址空间
    if(current->flags & PF_VFORK){
        current->mm = (struct mm_struct*)kmalloc(sizeof(struct mm_struct),0);
        memset(current->mm,0,sizeof(struct mm_struct));
        current->mm->pgd = (pml4t_t*)Virt_To_Phy(kmalloc(PAGE_4K_SIZE,0));
        color_printk(RED,BLACK,"load_binary_file malloc new pgd:%#018x\n",current->mm->pgd);
        memset(Phy_To_Virt(current->mm->pgd),0,PAGE_4K_SIZE/2); //前256项
        memcpy(Phy_To_Virt(current->mm->pgd)+256,Phy_To_Virt(init_task[SMP_cpu_id()]->mm->pgd),PAGE_4K_SIZE/2);
    }



    tmp = Phy_To_Virt((unsigned long *)((unsigned long)current->mm->pgd & (~0xfffUL)) +
                        ((code_start_address >> PAGE_GDT_SHIFT) & 0x1ff));
    virtual = kmalloc(PAGE_4K_SIZE, 0);
    set_pml4t(tmp, mk_pml4t(Virt_To_Phy(virtual), PAGE_USER_GDT));
    // set_pml4t(tmp, mk_pml4t(Virt_To_Phy(virtual), PAGE_KERNEL_GDT));

    tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) +
                        ((code_start_address >> PAGE_1G_SHIFT) & 0x1ff));
    virtual = kmalloc(PAGE_4K_SIZE, 0);
    set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_USER_Dir));
    // set_pdpt(tmp, mk_pdpt(Virt_To_Phy(virtual), PAGE_KERNEL_Dir));

    tmp = Phy_To_Virt((unsigned long *)(*tmp & (~0xfffUL)) +
                        ((code_start_address >> PAGE_2M_SHIFT) & 0x1ff));
    if(*tmp == NULL){
        p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped);
        set_pdt(tmp, mk_pdt(p->PHY_address, PAGE_USER_Page));
    }


    flush_tlb();

    if (!(current->flags & PF_KTHREAD)) {
        /*  
        进程的地址空间范围(不能超越)

        */
        current->addr_limit = STACK_SIZE;
    }
    current->mm->start_code = code_start_address;
    current->mm->end_code = 0;
    current->mm->start_data = 0;
    current->mm->end_data = 0;
    current->mm->start_rodata = 0;
    current->mm->end_rodata = 0;
    current->mm->start_bss = 0;
    current->mm->end_bss = 0;
    current->mm->start_brk = 0;
    current->mm->end_brk = 0;
    current->mm->start_stack = stack_start_address;

    exit_files(current);
    current->flags &= -PF_VFORK;

    filep = open_exec_file(name);

    /*  加载文件内容到内存中  */
    memset((void*)0x800000,0,PAGE_2M_SIZE);
    retval = filep->f_ops->read(filep,(void*)0x800000,filep->dentry->dir_inode->file_size,&pos);
    return retval;
}



void user_level_function() {
    int errno = 0;
    stop();
    char string[] = "Test system call";
    asm volatile(
        "pushq %%r10        \n\t"
        "pushq %%r11        \n\t"

        "leaq sysexit_return_address(%%rip),%%r10      \n\t"
        "movq %%rsp,%%r11   \n\t"
        "sysenter           \n\t"
        "sysexit_return_address: \n\t"

        "xchgq %%rdx,%%r10      \n\t"
        "xchgq %%rcx,%%r11      \n\t"

        "popq %%r11         \n\t"
        "popq %%r10         \n\t"
        : "=a"(errno) // rax存储系统调用号和返回值
        : "0"(__NR_putstring), "D"(string)
        : "memory");
    // color_printk(RED,BLACK,"user_level_function task called sysenter,errno:%ld\n",errno);
    // print_cr0_info();//无法在用户态执行该函数

    while (1);
}




/*  根据pid查找进程的pcb  */
struct task_struct* get_task(long pid){
    struct task_struct* task = NULL;
    for(task=init_task_union.task.next;task!=&init_task_union.task;task=task->next){
        if(task->pid == pid){
            return task;
        }
        __builtin_prefetch(task->next,0,3);
    }
    return NULL;
}



void wakeup_process(struct task_struct* task){
    task->state = TASK_RUNNING;
    insert_task_queue(task);
    current->flags |= NEED_SCHEDULE;
}



unsigned long copy_flags(unsigned long clone_flags,struct task_struct* task){
    if(clone_flags & CLONE_VM){
        task->flags |= PF_VFORK;
    }
    return 0;
}



unsigned long copy_files(unsigned long clone_flags,struct task_struct* task){
    int error = 0;
    if(clone_flags & CLONE_FS){
        goto out;
    }
    for (int i = 0; i < TASK_FILE_MAX;i++){
        if(current->file_struct[i] != NULL){
            task->file_struct[i] = (struct file *)kmalloc(sizeof(struct file), 0);
            memcpy(task->file_struct[i], current->file_struct[i], sizeof(struct file));
        }
    }
    out:
        return error;
}



void exit_files(struct task_struct* task){
    if(task->flags & PF_VFORK){
        ;
    }else{
        for (int i = 0; i < TASK_FILE_MAX;i++){
            kfree(task->file_struct[i]);
        }
    }
    memset(task->file_struct, 0, sizeof(struct file *) * TASK_FILE_MAX);
}



unsigned long copy_mm(unsigned long clone_flags,struct task_struct* task){
    int error = 0;
    struct mm_struct *newmm = NULL;
    unsigned long code_start_address = 0x800000;
    unsigned long stack_start_address = 0xa00000;
    unsigned long *tmp;
    unsigned long* virtual = NULL;
    struct Page *p = NULL;

    // 子进程共享父进程的地址空间 vfork
    if (clone_flags & CLONE_VM){
        newmm = current->mm;    //此时不需要新建一个mm,直接用父进程的就好了
        goto out;
    }
    newmm = (struct mm_struct *)kmalloc(sizeof(struct mm_struct),0);
    memcpy(newmm,current->mm,sizeof(struct mm_struct));
    newmm->pgd = (pml4t_t*)Virt_To_Phy(kmalloc(PAGE_4K_SIZE,0));
    // copy内核空间
    memcpy(Phy_To_Virt(newmm->pgd)+256,Phy_To_Virt(init_task[SMP_cpu_id()]->mm->pgd)+256,PAGE_4K_SIZE/2);
    // copy用户空间代码段、数据段、bss段空间
    memset(Phy_To_Virt(newmm->pgd),0,PAGE_4K_SIZE/2);

    tmp = Phy_To_Virt((unsigned long*) ((unsigned long)newmm->pgd & (~0xfffUL) + (code_start_address >> PAGE_GDT_SHIFT) & 0x1ff));
    virtual = kmalloc(PAGE_4K_SIZE,0);
    memset(virtual,0,PAGE_4K_SIZE);
    set_pml4t(tmp,mk_pml4t(Virt_To_Phy(virtual),PAGE_USER_GDT));


    tmp = Phy_To_Virt((unsigned long*)((unsigned long)*tmp & (~0xfffUL) + (code_start_address >> PAGE_1G_SHIFT) & 0x1ffUL));
    virtual = kmalloc(PAGE_4K_SIZE,0);
    memset(virtual,0,PAGE_4K_SIZE);
    set_pdpt(tmp,mk_pdpt(Phy_To_Virt(virtual),PAGE_USER_Dir));


    tmp = Phy_To_Virt((unsigned long*)((unsigned long)*tmp & 0xfffUL) + ((code_start_address >> PAGE_2M_SHIFT) & 0x1ffUL));
    p = alloc_pages(ZONE_NORMAL,1,PG_PTable_Maped);
    set_pdt(tmp,mk_pdt(p->PHY_address,PAGE_USER_Page));

    memcpy(Phy_To_Virt(p->PHY_address),(void*)code_start_address,stack_start_address - code_start_address);

out:
    task->mm = newmm;
    return error;
}



void exit_mm(struct task_struct* task){
    unsigned long code_start_address = 0x800000;
    unsigned long *tmp2,*tmp3,*tmp4;

    if(task->flags & PF_VFORK){
        return;
    }
    if(task->mm->pgd != NULL){
        // 释放该进程cr3所指的空间
        tmp4 = Phy_To_Virt((unsigned long*)((unsigned long)task->mm->pgd & (~0xfffUL) + (code_start_address >> PAGE_GDT_SHIFT) & 0xfffUL));

        tmp3 = Phy_To_Virt((unsigned long*)((unsigned long)*tmp4 & (~0xfffUL) + (code_start_address >> PAGE_1G_SHIFT) & 0x1ffUL));

        tmp2 = Phy_To_Virt((unsigned long*)((unsigned long)*tmp3 &(~0xfffUL) + (code_start_address >> PAGE_2M_SHIFT) & 0x1ff));

        free_pages(Phy_To_2M_Page(*tmp2),1);
        kfree(Phy_To_Virt(*tmp4));
        kfree(Phy_To_Virt(*tmp3));
        kfree(Phy_To_Virt(task->mm->pgd));

    }
    if(task->mm != NULL){
        kfree(task->mm);
    }
}



unsigned long copy_thread(
    unsigned long clone_flags,
    unsigned long stack_start,
    unsigned long stack_size,
    struct task_struct* task,
    struct pt_regs* regs
){
    struct thread_struct* thread = NULL;
    struct pt_regs* childregs = NULL;

    thread = (struct thread_struct*)(task + 1);
    memset(thread,0,sizeof(struct thread_struct));
    task->thread = thread;

    childregs = (struct pt_regs*)((unsigned long)task + STACK_SIZE) -1;
    memcpy(childregs,regs,sizeof(struct pt_regs));

    // fork函数在子进程中的返回值rax
    childregs->rax = 0;
    // 子进程的应用层栈指针
    childregs->rsp = stack_start;

    thread->rsp0 = (unsigned long)task + STACK_SIZE;
    thread->rsp = (unsigned long)childregs;
    thread->fs = current->thread->fs;
    thread->gs = current->thread->gs;


    if(task->flags & PF_KTHREAD){
        thread->rip = (unsigned long)kernel_thread_func;
    }else{
        thread->rip = (unsigned long)ret_system_call;
    }


    return 0;
}



void exit_thread(struct task_struct* task){

}


struct file* open_exec_file(char* path){
    struct dir_entry* dentry = NULL;
    struct file* filep = NULL;
    dentry = path_walk(path,0);
    if(dentry == NULL){
        return (void*)-ENOENT;
    }
    if(dentry->dir_inode->attribute == FS_ATTR_DIR){
        return (void*)-ENOTDIR;
    }
    filep = (struct file*)kmalloc(sizeof(struct file),0);
    if(filep == NULL){
        return (void*)-ENOMEM;
    }
    filep->position = 0;
    filep->mode = 0;
    filep->dentry = dentry;
    filep->f_ops = O_RDONLY;
    return filep;
}