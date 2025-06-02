#include "lib.h"
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
#include <spinlock.h>
#include <log.h>
#include <control.h>
#include <fat32.h>


#if PIC_APIC
#include <APIC.h>
#else
#endif


extern unsigned long _stack_start;

unsigned char buffer[512];

void init_all(void){
    load_TR(10);
    set_tss64((unsigned int*)&init_tss[0],_stack_start,_stack_start,_stack_start,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00);
    screen_init();
    sys_vector_init();
    cpu_init();
    memory_init();
    slab_init();
    /*  Slab初始化之后才有kmalloc  */
    unsigned char* ptr = NULL;
    ptr = (unsigned char*)kmalloc(STACK_SIZE,0) + STACK_SIZE;
    ((struct task_struct*)(ptr - STACK_SIZE))->cpu_id = 0;
    init_tss[0].ist1 = (unsigned long)ptr;
    init_tss[0].ist2 = (unsigned long)ptr;
    init_tss[0].ist3 = (unsigned long)ptr;
    init_tss[0].ist4 = (unsigned long)ptr;
    init_tss[0].ist5 = (unsigned long)ptr;
    init_tss[0].ist6 = (unsigned long)ptr;
    init_tss[0].ist7 = (unsigned long)ptr;

    frame_buffer_init();
    pagetable_init();
    IC_8259A_init();

#if PIC_APIC
    color_printk(RED,BLACK,"current PIC is APIC\n");
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
    screen_clear();
    SMP_init();
    softirq_init();
    timer_init();
    task_init();
    HPET_init();
    screen_clear();
    sti();

    //stop();
}