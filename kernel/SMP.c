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


struct INT_CMD_REG  icr_entry;
static spinlock_T SMP_lock;

int global_i = 0;

extern unsigned long _stack_start;
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


    wrmsr(0x830,0xc4500 );  //INIT IPI


    for(global_i = 1;global_i<8;global_i++){
        spin_lock(&SMP_lock);

        _stack_start = (unsigned long)kmalloc(STACK_SIZE,0 ) + STACK_SIZE;
        tss = (unsigned int*)kmalloc(128,0 ); //额外留下一部分空间

        set_tss64_descriptor((10 + global_i * 2), tss);

        set_tss64(tss,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start,_stack_start );

        icr_entry.vector = 0x20;
        icr_entry.deliver_mode = ICR_DELIVER_MODE_START_UP ;
        icr_entry.dest_shorthand = ICR_SHORTHAND_NONE;
        icr_entry.destination.x2apic_destination = global_i;


        wrmsr(0x830,*(unsigned long*)&icr_entry );
        wrmsr(0x830,*(unsigned long*)&icr_entry );  //向目标处理器投递两次Start UP
    }
    for(int i=200;i<210;i++){
        set_intr_gate(i,2 ,SMP_interrupt[i - 200] );
    }
    memset(SMP_IPI_desc,0,sizeof(irq_desc_T) * 10);
}



/*  由AP核在执行  */
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

    load_TR((10 + (global_i - 1) * 2));

    color_printk(GREEN,BLACK ,"CPU%d is running!\n",global_i - 1 );
    spin_unlock(&SMP_lock);


    hlt();
}