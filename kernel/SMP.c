#include "ptrace.h"
#include <printk.h>
#include <string.h>
#include <SMP.h>
#include <cpu.h>
#include <lib.h>
#include <APIC.h>
#include <spinlock.h>
#include <gate.h>
#include <memory.h>
#include <task.h>
#include <interrupt.h>
#include <schedule.h>
#include <screen.h>


struct INT_CMD_REG  icr_entry;
spinlock_T SMP_lock;

int global_i = 0;

extern unsigned long _stack_start;  //head.S中定义了该变量
unsigned int* tss;


void SMP_init(void){
    unsigned int a,b,c,d;
    for(int i=0;;i++){
        get_cpuid(0xb,i ,&a ,&b ,&c ,&d );
        if(((c >> 8) & 0xff) == 0){
            break;
        }
        color_printk(WHITE,BLACK,"Local APIC ID Package_../Core_2/SMT_1,type(%#lx)Width:%#lx,num of logical processor(%#lx)\n",c >> 8 & 0xff,a & 0x1f,b & 0xff);
    }
    color_printk(WHITE,BLACK ,"x2APIC ID level:%#lx\tx2APIC ID the current logical processor:%#lx\n",c & 0xff,d );

    color_printk(WHITE,BLACK ,"SMP copy byte:%#lx\n",(unsigned long)&_APU_boot_end - (unsigned long)&_APU_boot_start );

    spin_init(&SMP_lock);


    /*  把AP核心的启动引导写入到指定地址  */
    memcpy((unsigned char*)0xffff800000020000,_APU_boot_start ,(unsigned long)&_APU_boot_end - (unsigned long)&_APU_boot_start );



    struct INT_CMD_REG icr_entry;
    icr_entry.vector = 0x00;
    icr_entry.deliver_mode = ICR_DELIVER_MODE_INIT;
    icr_entry.dest_mode = ICR_DEST_MODE_PHY;
    icr_entry.deliver_status = ICR_DELIVER_STATUS_IDLE;
    icr_entry.level = ICR_LVEL_MODE_DE_ASSERT;
    icr_entry.trigger_mode = ICR_TRIGGER_MODE_EDGE;
    icr_entry.dest_shorthand = ICR_SHORTHAND_ALL_NOT_SELF;  //除自身外
    icr_entry.destination.x2apic_destination = 0x00;
    icr_entry.res_1 = icr_entry.res_2 = icr_entry.res_3 = 0;

    unsigned char* ptr = NULL;

    wrmsr(0x830,0xc4500 );  //INIT IPI


    for(global_i = 1;global_i<NR_CPUS;global_i++){
        spin_lock(&SMP_lock);

        ptr = (unsigned char*)kmalloc(STACK_SIZE,0 );
        ((struct task_struct *)ptr)->cpu_id = global_i;
        _stack_start = (unsigned long)ptr + STACK_SIZE;

        memset(&init_tss[global_i],0,sizeof(struct tss_struct));


        init_tss[global_i].rsp0 = _stack_start;
        init_tss[global_i].rsp1 = _stack_start;
        init_tss[global_i].rsp2 = _stack_start;
        color_printk(RED,BLACK ,"_stack_start:%#lx",_stack_start );
        ptr = (unsigned char*)kmalloc(STACK_SIZE,0 ) + STACK_SIZE;
        ((struct task_struct *)(ptr - STACK_SIZE))->cpu_id = global_i;

        init_tss[global_i].ist1 = (unsigned long)ptr;
        init_tss[global_i].ist2 = (unsigned long)ptr;
        init_tss[global_i].ist3 = (unsigned long)ptr;
        init_tss[global_i].ist4 = (unsigned long)ptr;
        init_tss[global_i].ist5 = (unsigned long)ptr;
        init_tss[global_i].ist6 = (unsigned long)ptr;
        init_tss[global_i].ist7 = (unsigned long)ptr;

        set_tss64_descriptor((10 + global_i * 2), &init_tss[global_i]);


        icr_entry.vector = 0x20;    //指定AP核心从什么位置开始运行
        icr_entry.deliver_mode = ICR_DELIVER_MODE_START_UP ;
        icr_entry.dest_shorthand = ICR_SHORTHAND_NONE;
        icr_entry.destination.x2apic_destination = global_i;


        wrmsr(0x830,*(unsigned long*)&icr_entry );
        wrmsr(0x830,*(unsigned long*)&icr_entry );  //向目标处理器投递两次Start UP
    }

    for(int i=200;i<210;i++){
        set_intr_gate(i,0 ,SMP_interrupt[i - 200] );
    }
    memset(SMP_IPI_desc,0,sizeof(irq_desc_T) * 10);
    register_IPI(200,NULL ,&IPI_0x200 ,0 ,NULL ,"IPI 0x200" );

}



/*  由AP核在执行(AP核心进入长模式后直接跳过来)  */
void Start_SMP(void){
    unsigned int x,y;
    color_printk(YELLOW,BLACK ,"APU starting......\n" );

    unsigned long msr = rdmsr(IA32_APIC_BASE_MSR);
    msr |= 0b110000000000;  //开启x2APIC模式
    //msr ^= 0b100000000;
    wrmsr(IA32_APIC_BASE_MSR,msr );


    msr = rdmsr(IA32_APIC_SVR_MSR);
    msr |= 0b0000100000000;  //开启APIC模式(bochs模拟器不支持禁用EOI)
    wrmsr(IA32_APIC_SVR_MSR,msr );

    unsigned long APIC_ID = rdmsr(IA32_APIC_ID_MSR );
    color_printk(GREEN,BLACK ,"APIC ID:%#lx\n",APIC_ID );

    color_printk(GREEN,BLACK ,"CPU%d is running!\n",global_i - 1 );

    interrupt_init();

    current->state = TASK_RUNNING;
    current->flags = PF_KTHREAD;
    current->mm = &init_mm;

    list_init(&current->list);
    current->addr_limit = 0xffff800000000000;
    current->priority = 2;
    current->virtual_runtime = 0;

    current->thread = (struct thread_struct *)(current + 1);
    memset(current->thread,0,sizeof(struct thread_struct));
    current->thread->rsp0 = _stack_start;
    current->thread->rsp = _stack_start;
    current->thread->fs = KERNEL_DS;
    current->thread->gs = KERNEL_DS;


    init_task[SMP_cpu_id()] = current;  //保存到init_task数组里

    load_TR((10 + (global_i - 1) * 2));
    spin_unlock(&SMP_lock);

    current->preempt_count = 0;
    sti();

    //task_init();



    while(1){
        hlt();
    }
}




void IPI_0x200(unsigned long nr,unsigned long parameter,struct pt_regs* regs){
    switch (current->priority) {
        case 0:
        case 1:
            task_schedule[SMP_cpu_id()].CPU_exec_task_jiffies--;
            current->virtual_runtime++;
            break;
        case 2:
        default:
            task_schedule[SMP_cpu_id()].CPU_exec_task_jiffies -= 2;
            current->virtual_runtime += 2;
            break;
    }
    if(task_schedule[SMP_cpu_id()].CPU_exec_task_jiffies <= 0){
        //current->flags |= NEED_SCHEDULE;
    }
}