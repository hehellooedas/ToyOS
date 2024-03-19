#ifndef __DEVICE_APIC_H
#define __DEVICE_APIC_H


#include <lib.h>
#include <ptrace.h>
#include <interrupt.h>


/*
 *
 *
 *
*/


/*  LVT结构(只有定时器有timer_mode)  */
struct APIC_LVT{
    unsigned int vector:8,
                 deliver_mode:3,
                 res_1:1,
                 deliver_status:1,
                 polarity:1,
                 irr:1,
                 trigger_mode:1,
                 mask:1,
                 timer_mode:2,
                 res_2:13;
}__attribute__((packed));

/*  APIC的投递模式表(deliver_mode)  */
#define APIC_LVT_DELIVER_MODE_FIXED     0b000   //由LVT寄存器的向量号区域指定中断向量号
#define APIC_LVT_DELIVER_MODE_SMI       0b010   //通过处理器的SMI信号线向处理器投递SMI(向量号区域必须为0)
#define APIC_LVT_DELIVER_MODE_NMI       0b100   //向处理器投递NMI请求(忽略向量号)
#define APIC_LVT_DELIVER_MODE_INT       0b101
#define APIC_LVT_DELIVER_MODE_ExtINT    0b111   //接受类8259A的中断请求

/*  APIC的投递状态(deliver_status)  */
#define APIC_LVT_DELIVER_STATUS_IDLE    0       //目前中断源没有或投递给处理器后已受理









struct IO_APIC_RET_ENTRY{

}__attribute__((packed));





struct IOAPIC_map{
    unsigned int physical_address;        //间接访问寄存器的物理基地址
    unsigned char* virtual_index_address; //索引寄存器(IOREGSEL)
    unsigned int* virtual_data_address;   //数据寄存器(IOWIN)
    unsigned int* virtual_EOI_address;    //EOI寄存器
}ioapic_map;  //需要一个全局变量来存储I/O APIC的"寄存器"





static __attribute__((always_inline))
unsigned int ioapic_read(unsigned char index)
{
    *ioapic_map.virtual_index_address = index;
    lfence();
    return (unsigned int)*ioapic_map.virtual_data_address;
}


static __attribute__((always_inline))
void ioapic_write(unsigned char index,unsigned int value)
{
    *ioapic_map.virtual_index_address = index;
    mfence();
    *ioapic_map.virtual_data_address = value;
    mfence();
}




/*  RTE类寄存器为64字节读写分别需要两次  */
static __attribute__((always_inline))
unsigned long ioapic_rte_read(unsigned char index)
{
    unsigned long ret;
    *ioapic_map.virtual_index_address = index + 1;
    lfence();
    ret = *ioapic_map.virtual_data_address;
    ret <<= 32;
    mfence();

    *ioapic_map.virtual_index_address = index;
    lfence();
    ret |= *ioapic_map.virtual_data_address;
    mfence();

    return ret;
}



static __attribute__((always_inline))
void ioapic_rte_write(unsigned char index,unsigned long value)
{
    *ioapic_map.virtual_index_address = index;
    mfence();
    *ioapic_map.virtual_data_address = value & 0xffffffff;
    value >>= 32;
    mfence();

    *ioapic_map.virtual_index_address = index + 1;
    mfence();
    *ioapic_map.virtual_data_address = value & 0xffffffff;
    mfence();
}



void Local_APIC_init(void);
void IOAPIC_init(void);
void APIC_IOAPIC_init(void);

#endif