#include <printk.h>
#include <screen.h>
#include <init.h>
#include <memory.h>
#include <trap.h>
#include <interrupt.h>
#include <cpu.h>
#include <gate.h>
#include <task.h>
#include <8259A.h>
#include <keyboard.h>
#include <mouse.h>
#include <disk.h>
#include <SMP.h>
#include <time.h>
#include <HPET.h>
#include <timer.h>
#include <softirq.h>
#include <schedule.h>


#if PIC_APIC
#include <APIC.h>
#else
#endif


extern unsigned long _stack_start;


void init_all(void){
    load_TR(10);
    set_tss64(TSS64_Table,_stack_start,_stack_start,_stack_start,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00);
    screen_init();
    sys_vector_init();
    cpu_init();
    memory_init();
    slab_init();
    frame_buffer_init();
    pagetable_init();
    IC_8259A_init();

#if PIC_APIC
    color_printk(RED,BLACK,"current PIC is APIC\n",APIC);
    APIC_IOAPIC_init();
#else
    color_printk(RED,BLACK,"current PIC is 8259A\n");
#endif
    interrupt_init();
    keyboard_init();
    mouse_init();
    disk_init();
    print_current_time();
    schedule_init();
    softirq_init();
    timer_init();
    HPET_init();    //在定时器产生中断之前,把其他模块都初始化完
    sti();
    task_init();
}