#include <io.h>
#include <APIC.h>
#include <printk.h>
#include <cpu.h>
#include <memory.h>
#include <lib.h>
#include <interrupt.h>
#include <stdbool.h>
#include <SMP.h>
#include <ptrace.h>
#include <interrupt.h>
#include <screen.h>


/*  ESR错误内容记录在这里  */


/*
*CPU的每个核心各拥有一个Local APIC(位于处理器内部)
*能够接收I/O APIC和其他处理器发送来的中断请求
*寄存器映射到物理地址空间(可以直接通过MSR寄存器访问)
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
    msr |= 0b0000100000000;  //开启APIC模式(bochs模拟器不支持禁用EOI)
    wrmsr(IA32_APIC_SVR_MSR,msr );


    unsigned long APIC_ID = rdmsr(IA32_APIC_ID_MSR );
    color_printk(GREEN,BLACK ,"APIC ID:%#lx\n",APIC_ID );
    unsigned long APIC_VERSION = rdmsr(IA32_APIC_VERSION_MSR);
    color_printk(GREEN,BLACK ,"APIC VERSION:%#lx  ",APIC_VERSION );
    if((APIC_VERSION & 0xff) > 0x10){
        color_printk(GREEN,BLACK ,"Integrated APIC\n" );
    }
    color_printk(GREEN,BLACK ,"MAX_LVT_ENTRY:%#lx,SVR(Suppress EOI Boardcast):%#lx\n",(APIC_VERSION >> 16 & 0xff) + 1,(APIC_VERSION  >> 24 & 0x1) );


    /*  屏蔽LVT的所有中断投递功能  */
    wrmsr(LVT_CMCI_MSR,0x10000 );
    wrmsr(LVT_TIMER_MSR,0x10000 );
    wrmsr(LVT_THERMAL_MSR,0x10000 );
    wrmsr(LVT_PERFORMANCE_MSR,0x10000 );
    wrmsr(LVT_LINT0_MSR,0x10000 );
    wrmsr(LVT_LINT1_MSR,0x10000 );
    wrmsr(LVT_ERROR_MSR,0x10000 );


    unsigned int TPR,PPR;
    TPR = (unsigned int)(rdmsr(TPR_MSR) & 0xffffffff);
    PPR = (unsigned int)(rdmsr(PPR_MSR) & 0xffffffff);
    color_printk(GREEN,BLACK ,"TPR=%#lx,PPR=%#lx\n",TPR,PPR );
}




/*
 * I/O APIC收集外设的中断请求 -> 封装成中断消息 -> 投递给CPU
 * 访问I/O APIC需要通过内存地址(访问的内存地址必须经过页表映射)
 * IOREGSEL寄存器:通过索引指定要读写的寄存器,低8位有效
 * IOWIN寄存器:用于读写目标寄存器里的数据,4B
*/
void IOAPIC_pagetable_remap(void)
{
    unsigned long* tmp;
    unsigned char* IOAPIC_addr = (unsigned char*)Phy_To_Virt(0xfec00000);

    ioapic_map.physical_address = 0xfec00000;
    ioapic_map.virtual_index_address = IOAPIC_addr;
    ioapic_map.virtual_data_address = (unsigned int*)(IOAPIC_addr + 0x10);
    ioapic_map.virtual_EOI_address = (unsigned int*)(IOAPIC_addr + 0x40);

    Global_CR3 = Get_gdt();

    tmp = Phy_To_Virt(Global_CR3 + (((unsigned long)IOAPIC_addr >> PAGE_GDT_SHIFT) & (0x1ff)));
    if(*tmp == 0){
        unsigned long* virtual = kmalloc(PAGE_4K_SIZE,0 );
        set_pml4t(tmp,mk_pml4t(Virt_To_Phy(virtual),PAGE_KERNEL_GDT ) );
    }
    tmp = Phy_To_Virt((unsigned long*)(*tmp & (~0xfffUL)) + (((unsigned long)IOAPIC_addr >>PAGE_1G_SHIFT) & 0x1ff));
    if(*tmp == 0){
        unsigned long* virtual = kmalloc(PAGE_4K_SIZE,0 );
        set_pdpt(tmp,mk_pdpt(Virt_To_Phy(virtual),PAGE_KERNEL_Dir ) );
    }
    tmp = Phy_To_Virt((unsigned long*)(*tmp & (~0xfffUL)) + (((unsigned long)IOAPIC_addr >> PAGE_2M_SHIFT) & 0x1ff));
    set_pdt(tmp,mk_pdt(ioapic_map.physical_address,PAGE_KERNEL_Page | PAGE_PWT | PAGE_PCD ) );

    flush_tlb();

}




void IOAPIC_init(void)
{
    ioapic_write(0,0x0f000000 );
    unsigned int IOAPIC_ID = ioapic_read(0);
    unsigned int IOAPIC_VERSION = ioapic_read(1);
    color_printk(GREEN,BLACK ,"IOAPIC ID:%#lx,IOAPIC VERSION:%#lx,MAX_RTE_ENTRY:%#lx\n",IOAPIC_ID,IOAPIC_VERSION & 0xff,((IOAPIC_VERSION >> 16) & 0xff) + 1);

    /*  初始化RTE表项  */
    for(int i=0x10;i<0x40;i+=2){
        ioapic_rte_write(i,0x10020 + ((i - 0x10) >> 1 ));
    }
    ioapic_rte_write(0x12,0x21 );  //接收键盘中断
}





void APIC_IOAPIC_init(void)
{

    IOAPIC_pagetable_remap();

    /*  屏蔽8259A中断控制器  */
    out8(0x21,0xff);
    out8(0xa1,0xff);

    /*  CPU只接收APIC的中断请求信号  */
    out8(0x22,0x70 );
    out8(0x23,0x01 );

    Local_APIC_init();
    screen_clear();
    //SMP_init();
    IOAPIC_init();

    memset(interrupt_desc,0,sizeof(irq_desc_T) *NR_IRQS);
}



void IOAPIC_enbale(unsigned long irq)
{
    /*  irq号中断对应的I/O中断投递寄存器(绕过前0x10)  */
    unsigned long value = ioapic_rte_read((irq - 0x20) * 2 + 0x10);
    value &= (~0x10000UL);  //解除屏蔽
    ioapic_rte_write((irq - 0x20) * 2 + 0x10,value );
}



void IOAPIC_disable(unsigned long irq)
{
    unsigned long value = ioapic_rte_read((irq - 0x20) * 2 + 0x10);
    value |= (~0x10000UL);  //屏蔽
    ioapic_rte_write((irq - 0x20) * 2 + 0x10,value );
}



bool IOAPIC_install(unsigned long irq,void* arg)
{
    struct IO_APIC_RET_ENTRY* entry = (struct IO_APIC_RET_ENTRY*)arg;
    /*  把RTE结构体变成一个单独的数放入RTE寄存器  */
    ioapic_rte_write((irq - 0x20) * 2 + 0x10,*(unsigned long*)arg );
    return true;
}


void IOAPIC_uninstall(unsigned long irq)
{
    /*  卸载驱动:变回初始化时候的样子  */
    ioapic_rte_write((irq - 0x20) * 2 + 0x10,0x10000);
}



void IOAPIC_level_ack(unsigned long irq)
{
    wrmsr(0x80b,0 );
}


void IOAPIC_edge_ack(unsigned long irq)
{
    wrmsr(0x80b,0 );
}


void Local_APIC_edge_level_ack(unsigned long nr){
    wrmsr(0x80b,0 );
}

