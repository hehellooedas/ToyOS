#include "printk.h"
#include <screen.h>
#include <init.h>
#include <memory.h>
#include <trap.h>
#include <interrupt.h>
#include <cpu.h>
#include <gate.h>
#include <task.h>


#if PIC_APIC
#include <8259A.h>
#include <APIC.h>
#else
#include <8259A.h>
#endif



void init_all(void){
    load_TR(10);
    set_tss64(0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00,0xffff800000007c00);
    screen_init();
    sys_vector_init();
    cpu_init();
    memory_init();
    slab_init();
    frame_buffer_init();
    pagetable_init();
#if PIC_APIC
    IC_8259A_init();
    color_printk(RED,BLACK,"current PIC is APIC\n",APIC);
    APIC_IOAPIC_init();
#else
    color_printk(RED,BLACK,"current PIC is 8259A\n");
    IC_8259A_init();
#endif
    interrupt_init();
    color_printk(GREEN,BLACK,"bochs will run sti!\n");
    asm volatile ("nop  \n\t");
    sti();
    //task_init();
}