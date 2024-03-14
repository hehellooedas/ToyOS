#include <APIC.h>
#include <printk.h>
#include <cpu.h>
#include <lib.h>
#include <interrupt.h>



#define IA32_APIC_BASE_MSR          0x1B
#define IA32_APIC_BASE_MSR_BSP      0x100 // 处理器是 BSP
#define IA32_APIC_SVR_MSR           0x80f
#define IA32_APIC_ID_MSR            0x802
#define IA32_APIC_VERSION_MSR       0x803


/*  LVT本地中断向量表(处理器内部产生的中断请求)  */
#define LVT_CMCI_MSR        0x82f   //
#define LVT_TIMER_MSR       0x832   //定时器寄存器
#define LVT_THERMAL_MSR     0x833   //过热保护寄存器
#define LVT_PERFORMANCE_MSR 0x834   //性能监控计数寄存器
#define LVT_LINT0_MSR       0x835   //当处理器的LINT0引脚接收到中断请求信号时
#define LVT_LINT1_MSR       0x836   //当处理器的LINT1引脚接收到中断请求信号时
#define LVT_ERROR_MSR       0x837   //内部错误寄存器


#define TPR_MSR             0x808
#define PPR_MSR             0x80a



/*  ESR错误内容记录在这里  */


/*
CPU的每个核心各拥有一个Local APIC
能够接收I/O APIC和其他处理器发送来的中断请求
寄存器映射到物理地址空间(可以直接通过寄存器访问)
*/
void Local_APIC_init(void)
{
    unsigned long msr = rdmsr(IA32_APIC_BASE_MSR);
    color_printk(GREEN,BLACK ,"%#lx\n",msr );
    msr |= 0b110000000000;  //开启x2APIC模式
    //msr ^= 0b100000000;
    wrmsr(IA32_APIC_BASE_MSR,msr );
    msr = rdmsr(IA32_APIC_BASE_MSR);
    color_printk(GREEN,BLACK ,"%#lx\n",msr );


    msr = rdmsr(IA32_APIC_SVR_MSR);
    color_printk(GREEN,BLACK ,"%#lx\n",msr );
    msr |= 0b1000100000000;  //开启APIC模式
    //wrmsr(IA32_APIC_SVR_MSR,msr );
    color_printk(GREEN,BLACK ,"%#lx\n",msr );


    unsigned long APIC_ID = rdmsr(IA32_APIC_ID_MSR );
    color_printk(GREEN,BLACK ,"APIC ID:%#lx\n",APIC_ID );
    unsigned long APIC_VERSION = rdmsr(IA32_APIC_VERSION_MSR);
    color_printk(GREEN,BLACK ,"APIC VERSION:%#lx  ",APIC_VERSION );
    if((APIC_VERSION & 0xff) > 0x10){
        color_printk(GREEN,BLACK ,"Integrated APIC\n" );
    }
    color_printk(GREEN,BLACK ,"MAX_LVT_ENTRY:%#lx,SVR(Suppress EOI Boardcast):%#lx\n",(APIC_VERSION >> 16 & 0xff) + 1,(APIC_VERSION  >> 24 & 0x1) );


    /*  屏蔽LVT的所有中断投递功能  */
    //wrmsr(LVT_CMCI_MSR,0x10000 );
    //wrmsr(LVT_TIMER_MSR,0x10000 );
    //wrmsr(LVT_THERMAL_MSR,0x10000 );
    //wrmsr(LVT_PERFORMANCE_MSR,0x10000 );
    //wrmsr(LVT_LINT0_MSR,0x10000 );
    //wrmsr(LVT_LINT1_MSR,0x10000 );
    //wrmsr(LVT_ERROR_MSR,0x10000 );

    unsigned int TPR,PPR;
    TPR = (unsigned int)(rdmsr(TPR_MSR) & 0xffffffff);
    PPR = (unsigned int)(rdmsr(PPR_MSR) & 0xffffffff);
    color_printk(GREEN,BLACK ,"TPR=%#lx,PPR=%#lx\n",TPR,PPR );
}





void IOAPIC_init(void)
{

}





void APIC_IOAPIC_init(void)
{
    interrupt_init();

    /*  屏蔽8259A中断控制器  */
    out8(0x21,0xff);
    out8(0xa1,0xff);

    Local_APIC_init();
    while(1);
    sti();
}