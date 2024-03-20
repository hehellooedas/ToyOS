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


#if PIC_APIC
#include <APIC.h>
#else
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
    IC_8259A_init();
#if PIC_APIC
    color_printk(RED,BLACK,"current PIC is APIC\n",APIC);
    APIC_IOAPIC_init();
#else
    color_printk(RED,BLACK,"current PIC is 8259A\n");
#endif
    interrupt_init();
    color_printk(GREEN,BLACK,"bochs will run sti!\n");
    sti();
    keyboard_init();
    mouse_init();

    //task_init();
}