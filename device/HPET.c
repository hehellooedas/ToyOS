#include "schedule.h"
#include <printk.h>
#include <interrupt.h>
#include <memory.h>
#include <HPET.h>
#include <lib.h>
#include <softirq.h>
#include <timer.h>
#include <task.h>


extern unsigned long jiffies;

hw_int_controler HPET_int_controler = {
    .enable = IOAPIC_enbale,
    .disable = IOAPIC_disable,
    .installer = IOAPIC_install,
    .uninstaller = IOAPIC_uninstall,
    .ack = IOAPIC_edge_ack
};


void HPET_init(void)
{
    unsigned char* HPET_addr = (unsigned char*)Phy_To_Virt(0xfed00000);

    struct IO_APIC_RET_ENTRY entry;
    entry.vector = 34;
    entry.deliver_mode = APIC_DELIVER_MODE_FIXED;
    entry.dest_mode = APIC_DEST_MODE_PHY;
    entry.deliver_mode = APIC_DELIVER_STATUS_IDLE;
    entry.polarity = APIC_POLARITY_HIGH;
    entry.irr = APIC_IRR_RESET;
    entry.trigger_mode = APIC_TRIGGER_MODE_EDGE;
    entry.mask = APIC_MASKED;
    entry.res_1 = 0;
    entry.destination.physical.phy_dest = entry.destination.physical.res_2 = entry.destination.physical.res_3 = 0;

    register_irq(34,&entry ,&HPET_handler ,0 ,&HPET_int_controler ,"HPET" );

    color_printk(GREEN,BLACK,"HPET - GCAP_ID:%#lx\n",*(unsigned long*)HPET_addr);


    *(unsigned long*)(HPET_addr + HPET_GEN_CONF) = 3;
    mfence();

    *(unsigned long*)(HPET_addr + HPET_TIM0_CONF) = gen_time_conf(Timer_Conf_Interrupt_Enable | Timer_Conf_Modify_CNT_Enable | Timer_Conf_Trigger_Mode_Edge | Timer_Conf_Type_Cyclical);

    mfence();

    *(unsigned long*)(HPET_addr + HPET_TIM0_COMP) = 14318179;
    mfence();

    *(unsigned long*)(HPET_addr + HPET_MAIN_CNT) = 0;
    mfence();
}



void HPET_handler(unsigned long nr,unsigned long parameter,struct pt_regs* regs)
{
    jiffies++;
    if((container_of(get_List_next(&timer_list_head.list),struct timer_list ,list )->expire_jiffies <= jiffies))
        set_softirq_status(TIMER_STRQ);
    switch (current->priority) {
        case 0:
        case 1:
            task_schedule.CPU_exec_task_jiffies--;
            current->virtual_runtime++;
            break;
        case 2:
        default:
            task_schedule.CPU_exec_task_jiffies -= 2;
            current->virtual_runtime += 2;
            break;
    }
    if(task_schedule.CPU_exec_task_jiffies <= 0){
        current->flags |= NEED_SCHEDULE;
    }
}