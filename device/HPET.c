#include "SMP.h"
#include "schedule.h"
#include <printk.h>
#include <interrupt.h>
#include <memory.h>
#include <HPET.h>
#include <lib.h>
#include <softirq.h>
#include <string.h>
#include <timer.h>
#include <task.h>
#include <APIC.h>


extern struct schedule task_schedule[NR_CPUS];

struct GCAP_ID_REG GCAP_ID;


hw_int_controller HPET_int_controller = {
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

    register_irq(34,&entry ,&HPET_handler ,0 ,&HPET_int_controller ,"HPET" );


    GCAP_ID = *((struct GCAP_ID_REG*)(HPET_addr + HPET_GCAP_ID));
    color_printk(GREEN,BLACK,"HPET:OEM ID:%#x,counter_count:%d\n",GCAP_ID.OEM_ID,GCAP_ID.counter_count);


    *(unsigned long*)(HPET_addr + HPET_GEN_CONF) = 3;
    mfence();

    *(unsigned long*)(HPET_addr + HPET_TIM0_CONF) = gen_time_conf(Timer_Conf_Interrupt_Enable | Timer_Conf_Modify_CNT_Enable | Timer_Conf_Trigger_Mode_Edge | Timer_Conf_Type_Cyclical);

    mfence();

    *(unsigned long*)(HPET_addr + HPET_TIM0_COMP) = 1000000 / (GCAP_ID.accuracy / 1000000) * HPET_cycle;     //1s一次的时钟中断

    mfence();

    *(unsigned long*)(HPET_addr + HPET_MAIN_CNT) = 0;
    mfence();
}



void HPET_handler(unsigned long nr,unsigned long parameter,struct pt_regs* regs)
{

    jiffies++;

    /*  把时钟中断发送给其他核心  */

    struct INT_CMD_REG icr_entry;
    memset(&icr_entry,0,sizeof(struct INT_CMD_REG));    //没赋值的参数就是0
    icr_entry.vector = 0xc8;
    icr_entry.dest_shorthand = ICR_SHORTHAND_ALL_NOT_SELF;
    icr_entry.trigger_mode = ICR_TRIGGER_MODE_EDGE;
    icr_entry.dest_mode = ICR_DEST_MODE_PHY;
    icr_entry.deliver_mode = ICR_DELIVER_MODE_FIXED;
    wrmsr(0x830,*(unsigned long*)&icr_entry );


    if((container_of(get_List_next(&timer_list_head.list),struct timer_list ,list )->expire_jiffies <= jiffies))
        set_softirq_status(TIMER_STRQ);
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
        current->flags |= NEED_SCHEDULE;
    }
}